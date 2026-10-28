#include "jkfitness10800treadmill.h"

#include "jkfitness10800protocol.h"
#include "virtualdevices/virtualtreadmill.h"

#include <QSettings>

namespace {
constexpr quint16 SERIAL_PORT_UUID = 0x1101;
constexpr int SETUP_TIMEOUT_MS = 300;
constexpr int POLL_INTERVAL_MS = 500;
constexpr double MAX_SPEED = 22.0;
constexpr double MAX_INCLINE = 15.0;

bool isSerialPortService(const QBluetoothServiceInfo &service) {
    const QString name = service.serviceName();
    return service.serviceUuid() == QBluetoothUuid(SERIAL_PORT_UUID) ||
           name.startsWith(QStringLiteral("SerialPort"), Qt::CaseInsensitive) ||
           name.startsWith(QStringLiteral("Serial Port"), Qt::CaseInsensitive);
}

} // namespace

jkfitness10800treadmill::jkfitness10800treadmill(bool noConsole, bool noHeartService)
    : noConsole(noConsole), noHeartService(noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &jkfitness10800treadmill::update);
    refresh->start(200);

    setupTimeoutTimer.setSingleShot(true);
    connect(&setupTimeoutTimer, &QTimer::timeout, this, &jkfitness10800treadmill::setupTimeout);
}

jkfitness10800treadmill::~jkfitness10800treadmill() {
    setupTimeoutTimer.stop();
    if (discoveryAgent) {
        discoveryAgent->stop();
    }
    if (socket) {
        socket->disconnect(this);
        socket->close();
    }
}

double jkfitness10800treadmill::minStepInclination() {
    return 1.0;
}

double jkfitness10800treadmill::minStepSpeed() {
    return 0.01;
}

bool jkfitness10800treadmill::autoPauseWhenSpeedIsZero() {
    return lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > lastStart + 10000;
}

bool jkfitness10800treadmill::autoStartWhenSpeedIsGreaterThenZero() {
    return (lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > lastStop + 25000) && requestStop == -1;
}

void jkfitness10800treadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    emit debug(QStringLiteral("Found JK Fitness 10800 device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    if (discoveryAgent) {
        discoveryAgent->stop();
        discoveryAgent->deleteLater();
    }

    discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    discoveryAgent->setRemoteAddress(device.address());
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
            &jkfitness10800treadmill::serviceDiscovered);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this,
            &jkfitness10800treadmill::serviceFinished);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this,
            &jkfitness10800treadmill::serviceFinished);
    discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
    emit debug(QStringLiteral("RFCOMM service discovery started"));
}

void jkfitness10800treadmill::serviceDiscovered(const QBluetoothServiceInfo &service) {
    if (service.device().address() != bluetoothDevice.address() || !isSerialPortService(service)) {
        return;
    }

    serialPortService = service;
    emit debug(QStringLiteral("Serial Port service found: ") + service.serviceUuid().toString());
}

void jkfitness10800treadmill::createSocket() {
    if (socket) {
        return;
    }

    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(socket, &QBluetoothSocket::readyRead, this, &jkfitness10800treadmill::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, &jkfitness10800treadmill::rfCommConnected);
    connect(socket, &QBluetoothSocket::disconnected, this, &jkfitness10800treadmill::socketDisconnected);
    connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this,
            &jkfitness10800treadmill::onSocketErrorOccurred);
#ifdef Q_OS_ANDROID
    socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif
}

void jkfitness10800treadmill::serviceFinished() {
    createSocket();
    if (socket->state() != QBluetoothSocket::UnconnectedState) {
        return;
    }

    if (serialPortService.isValid()) {
        emit debug(QStringLiteral("Connecting to JK Fitness RFCOMM Serial Port service"));
        socket->connectToService(serialPortService);
    } else {
        emit debug(QStringLiteral("Serial Port service was not reported; trying UUID 1101 directly"));
        socket->connectToService(bluetoothDevice.address(), QBluetoothUuid(SERIAL_PORT_UUID));
    }
}

void jkfitness10800treadmill::rfCommConnected() {
    emit debug(QStringLiteral("JK Fitness RFCOMM connected: ") + socket->peerName());
    resetConnectionState();
    const QVector<QByteArray> initialization = jkfitness10800protocol::initializationFrames();
    for (const QByteArray &frame : initialization) {
        setupQueue.enqueue(frame);
    }
    sendNextSetupFrame();
}

void jkfitness10800treadmill::sendNextSetupFrame() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState) {
        return;
    }

    if (setupQueue.isEmpty()) {
        finishSetup();
        return;
    }

    const QByteArray frame = setupQueue.dequeue();
    setupWaiting = true;
    writeFrame(frame, QStringLiteral("setup"));
    setupTimeoutTimer.start(SETUP_TIMEOUT_MS);
}

void jkfitness10800treadmill::setupTimeout() {
    if (!setupWaiting) {
        return;
    }

    setupWaiting = false;
    emit debug(QStringLiteral("setup response timeout; advancing to next frame"));
    sendNextSetupFrame();
}

void jkfitness10800treadmill::finishSetup() {
    if (initDone) {
        return;
    }

    if (!setupHadValidResponse) {
        emit debug(QStringLiteral("RFCOMM probe finished without validated telemetry; controls remain disabled"));
        return;
    }

    setupWaiting = false;
    setupTimeoutTimer.stop();
    initDone = true;
    emit debug(QStringLiteral("RFCOMM setup complete; hardware acceptance remains unverified"));
    if (!connectedSignalEmitted) {
        connectedSignalEmitted = true;
        emit connectedAndDiscovered();
    }
}

void jkfitness10800treadmill::writeFrame(const QByteArray &frame, const QString &info) {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState) {
        emit debug(QStringLiteral("RFCOMM write skipped because socket is not connected: ") + info);
        return;
    }

    emit debug(QStringLiteral(">> ") + frame.toHex(' ') + QStringLiteral(" // ") + info);
    socket->write(frame);
}

void jkfitness10800treadmill::readSocket() {
    if (!socket) {
        return;
    }

    socketReadBuffer.append(socket->readAll());
    const QVector<jkfitness10800protocol::Frame> frames =
        jkfitness10800protocol::consumeFrames(socketReadBuffer);
    for (const jkfitness10800protocol::Frame &frame : frames) {
        emit debug(QStringLiteral("<< ") + frame.raw.toHex(' ') + QStringLiteral(" // opcode=0x") +
                   QString::number(frame.opcode, 16));
        processFrame(frame.raw);

        if (setupWaiting) {
            setupTimeoutTimer.stop();
            setupWaiting = false;
            sendNextSetupFrame();
        }
    }
}

void jkfitness10800treadmill::processFrame(const QByteArray &frame) {
    bool running = false;
    if (jkfitness10800protocol::isConsoleState(frame, running)) {
        setupHadValidResponse = true;
        if (!runningStateKnown || runningState != running) {
            runningStateKnown = true;
            runningState = running;
            if (running) {
                lastStart = 0;
                emit tapeStarted();
                emit buttonHWStart();
            } else {
                lastStop = 0;
                emit buttonHWStop();
            }
        }
        if (!setupWaiting && setupQueue.isEmpty()) {
            finishSetup();
        }
        return;
    }

    double speed = 0.0;
    double incline = 0.0;
    if (!jkfitness10800protocol::decodeTelemetry(frame, speed, incline)) {
        return;
    }

    setupHadValidResponse = true;

    const QDateTime now = QDateTime::currentDateTime();
    if (Speed.value() != speed) {
        emit speedChanged(speed);
    }
    Speed = speed;
    if (Inclination.value() != incline) {
        emit inclinationChanged(0.0, incline);
    }
    Inclination = incline;
    if (speed > 0.0) {
        lastSpeed = speed;
        lastInclination = incline;
        lastStart = 0;
    } else {
        lastStop = 0;
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed) +
               QStringLiteral(" incline: ") + QString::number(incline) +
               QStringLiteral(" telemetry=source-derived-13-byte"));
    lastTelemetryUpdate = now;
    if (!setupWaiting && setupQueue.isEmpty()) {
        finishSetup();
    }
}

void jkfitness10800treadmill::update() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState || !initDone) {
        return;
    }

    QSettings settings;
    if (!hasVirtualDevice() && settings.value(QZSettings::virtual_device_enabled,
                                               QZSettings::default_virtual_device_enabled)
                                      .toBool()) {
        emit debug(QStringLiteral("creating virtual treadmill interface..."));
        auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
        connect(virtualTreadmill, &virtualtreadmill::debug, this, &jkfitness10800treadmill::debug);
        connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                &jkfitness10800treadmill::changeInclinationRequested);
        setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
    }

    bool commandSent = false;
    if (requestSpeed != -1) {
        const double requestedInput = requestSpeed;
        const double requested = qBound(0.0, requestedInput, MAX_SPEED);
        requestSpeed = -1;
        if (qAbs(requested - currentSpeed().value()) >= minStepSpeed()) {
            writeFrame(jkfitness10800protocol::speedFrame(requested),
                       QStringLiteral("speed request=") + QString::number(requested));
            commandSent = true;
        }
    } else if (requestInclination != -100) {
        const double requestedInput = requestInclination;
        const double requested = qBound(0.0, requestedInput, MAX_INCLINE);
        requestInclination = -100;
        if (qAbs(requested - currentInclination().value()) >= minStepInclination()) {
            writeFrame(jkfitness10800protocol::inclineFrame(requested),
                       QStringLiteral("incline request=") + QString::number(requested));
            commandSent = true;
        }
    } else if (requestStart != -1) {
        requestStart = -1;
        writeFrame(jkfitness10800protocol::startFrame(), QStringLiteral("start"));
        emit tapeStarted();
        commandSent = true;
    } else if (requestStop != -1) {
        requestStop = -1;
        writeFrame(jkfitness10800protocol::stopFrame(), QStringLiteral("stop"));
        lastStop = QDateTime::currentMSecsSinceEpoch();
        commandSent = true;
    }

    if (!commandSent && pollCounter++ >= (POLL_INTERVAL_MS / refresh->interval())) {
        pollCounter = 0;
        writeFrame(jkfitness10800protocol::keepAliveFrame(), QStringLiteral("poll/keep-alive"));
    }

    update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));
}

void jkfitness10800treadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0.0) {
        percentage = 0.0;
    }
    changeInclination(grade, percentage);
}

void jkfitness10800treadmill::socketDisconnected() {
    resetConnectionState();
    emit debug(QStringLiteral("JK Fitness RFCOMM disconnected"));
    if (!disconnectedSignalEmitted) {
        disconnectedSignalEmitted = true;
        emit disconnected();
    }
}

void jkfitness10800treadmill::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    emit debug(QStringLiteral("JK Fitness RFCOMM socket error=") + QString::number(error));
    socketDisconnected();
}

void jkfitness10800treadmill::resetConnectionState() {
    setupTimeoutTimer.stop();
    setupQueue.clear();
    socketReadBuffer.clear();
    setupWaiting = false;
    setupHadValidResponse = false;
    initDone = false;
    connectedSignalEmitted = false;
    disconnectedSignalEmitted = false;
    runningStateKnown = false;
    runningState = false;
    pollCounter = 0;
}

bool jkfitness10800treadmill::connected() {
    return socket && socket->state() == QBluetoothSocket::ConnectedState && initDone;
}
