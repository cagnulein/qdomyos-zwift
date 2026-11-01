#include "jkfitness10800treadmill.h"

#include "jkfitness10800protocol.h"
#include "virtualdevices/virtualtreadmill.h"

#include <QSettings>

namespace {
constexpr quint16 SERIAL_PORT_UUID = 0x1101;
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

    heartbeatTimer.setInterval(1000);
    connect(&heartbeatTimer, &QTimer::timeout, this, &jkfitness10800treadmill::sendHeartbeat);
    startupSettleTimer.setSingleShot(true);
    connect(&startupSettleTimer, &QTimer::timeout, this, [this]() {
        if (!socket || socket->state() != QBluetoothSocket::ConnectedState) return;
        const QVector<QByteArray> frames = jkfitness10800protocol::initializationFrames();
        emit debug(QStringLiteral("Sending native 27-frame protocol query sequence"));
        for (const QByteArray &frame : frames) writeFrame(frame, QStringLiteral("query"));
    });
    startSequenceTimer.setSingleShot(true);
    connect(&startSequenceTimer, &QTimer::timeout, this, &jkfitness10800treadmill::sendNextStartFrame);
}

jkfitness10800treadmill::~jkfitness10800treadmill() {
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
    sendHeartbeat();
    heartbeatTimer.start();
    startupSettleTimer.start(700);
}

void jkfitness10800treadmill::sendHeartbeat() {
    writeFrame(jkfitness10800protocol::heartbeatFrame(), QStringLiteral("native heartbeat"));
}

void jkfitness10800treadmill::sendNextStartFrame() {
    if (requestStop != -1) {
        startSequenceQueue.clear();
        startWaitingForRunning = false;
        return;
    }
    if (!connected() || startSequenceQueue.isEmpty()) {
        startSequenceQueue.clear();
        startWaitingForRunning = false;
        return;
    }
    const QByteArray frame = startSequenceQueue.dequeue();
    writeFrame(frame, QStringLiteral("native type-2 start sequence"));
    if (frame == QByteArray::fromHex("55080101")) {
        startWaitingForRunning = true;
        return;
    }
    if (!startSequenceQueue.isEmpty()) startSequenceTimer.start(600);
}

void jkfitness10800treadmill::updateReadyState() {
    if (!deviceTypeKnown || !unitsKnown || protocolDeviceType != 2 || connectedSignalEmitted) return;
    initDone = true;
    connectedSignalEmitted = true;
    emit connectedAndDiscovered();
}

void jkfitness10800treadmill::writeFrame(const QByteArray &frame, const QString &info) {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState) {
        emit debug(QStringLiteral("RFCOMM write skipped because socket is not connected: ") + info);
        return;
    }

    emit debug(QStringLiteral(">> ") + frame.toHex(' ') + QStringLiteral(" // ") + info);
    const qint64 written = socket->write(frame);
    if (written != frame.size()) {
        emit debug(QStringLiteral("RFCOMM write accepted ") + QString::number(written) + QStringLiteral(" of ") +
                   QString::number(frame.size()) + QStringLiteral(" bytes // ") + info);
    }
}

void jkfitness10800treadmill::readSocket() {
    if (!socket) {
        return;
    }

    const QByteArray chunk = socket->readAll();
    emit debug(QStringLiteral("RX chunk: ") + chunk.toHex(' '));
    socketReadBuffer.append(chunk);
    const QVector<jkfitness10800protocol::Frame> frames =
        jkfitness10800protocol::consumeFrames(socketReadBuffer);
    for (const jkfitness10800protocol::Frame &frame : frames) {
        emit debug(QStringLiteral("<< ") + frame.raw.toHex(' ') + QStringLiteral(" // opcode=0x") +
                   QString::number(frame.opcode, 16));
        processFrame(frame.raw);
    }
}

void jkfitness10800treadmill::processFrame(const QByteArray &frame) {
    bool running = false;
    bool paused = false;
    int discoveredValue = 0;
    if (jkfitness10800protocol::decodeDeviceType(frame, discoveredValue)) {
        protocolDeviceType = discoveredValue;
        deviceTypeKnown = true;
        emit debug(QStringLiteral("Protocol device type discovered: ") + QString::number(protocolDeviceType));
        if (protocolDeviceType != 2) {
            initDone = false;
            startSequenceTimer.stop();
            startSequenceQueue.clear();
            startWaitingForRunning = false;
            emit debug(QStringLiteral("Discovered PAFERS device type is not a treadmill; treadmill controls disabled"));
            return;
        }
        updateReadyState();
        if (unitsKnown) writeFrame(jkfitness10800protocol::telemetryQueryFrame(), QStringLiteral("telemetry query after discovery"));
        return;
    }
    if (jkfitness10800protocol::decodeUnits(frame, discoveredValue)) {
        units = discoveredValue;
        unitsKnown = true;
        emit debug(QStringLiteral("Protocol speed units discovered: ") + QString::number(units));
        updateReadyState();
        if (deviceTypeKnown && protocolDeviceType == 2) {
            writeFrame(jkfitness10800protocol::telemetryQueryFrame(), QStringLiteral("telemetry query after discovery"));
        }
        return;
    }
    if (jkfitness10800protocol::isRetryDeviceTypeQuery(frame)) {
        writeFrame(QByteArray::fromHex("55bb01ff"), QStringLiteral("retry device type query"));
        return;
    }
    const bool typeTwoStopped = deviceTypeKnown && protocolDeviceType == 2 && frame == QByteArray::fromHex("550b0101");
    if (jkfitness10800protocol::isConsoleState(frame, running, paused) || typeTwoStopped) {
        if (typeTwoStopped) running = false;
        if (running && startWaitingForRunning) {
            startWaitingForRunning = false;
            startSequenceTimer.start(3000);
        } else if (!running && startWaitingForRunning) {
            startWaitingForRunning = false;
            startSequenceTimer.stop();
            startSequenceQueue.clear();
        }
        // A stop reply while preparing the session can acknowledge our reset.
        // Once running (or explicitly paused), cancel all delayed start writes.
        if (!running && (runningState || paused) && !startSequenceQueue.isEmpty()) {
            startSequenceTimer.stop();
            startSequenceQueue.clear();
            startWaitingForRunning = false;
            requestStart = -1;
            requestSpeed = -1;
            requestInclination = -100;
        }
        if (!runningStateKnown || runningState != running || pausedState != paused) {
            runningStateKnown = true;
            runningState = running;
            pausedState = paused;
            if (running) {
                lastStart = 0;
                emit tapeStarted();
                emit buttonHWStart();
            } else if (paused) {
                emit debug(QStringLiteral("Console reports paused"));
            } else {
                lastStop = 0;
                emit buttonHWStop();
            }
        }
        return;
    }

    double speed = 0.0;
    double incline = 0.0;
    if (!deviceTypeKnown || !unitsKnown || protocolDeviceType != 2 ||
        !jkfitness10800protocol::decodeTelemetry(frame, protocolDeviceType, units, speed, incline)) {
        return;
    }

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
               QStringLiteral(" incline: ") + QString::number(incline) + QStringLiteral(" telemetry=source-derived"));
    lastTelemetryUpdate = now;
}

void jkfitness10800treadmill::update() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState || !initDone || !deviceTypeKnown ||
        !unitsKnown || protocolDeviceType != 2) {
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
    if (requestStop != -1) {
        requestStop = -1;
        requestStart = -1;
        requestSpeed = -1;
        requestInclination = -100;
        startSequenceTimer.stop();
        startSequenceQueue.clear();
        startWaitingForRunning = false;
        writeFrame(jkfitness10800protocol::stopFrame(), QStringLiteral("stop"));
        lastStop = QDateTime::currentMSecsSinceEpoch();
        commandSent = true;
    } else if (!startSequenceQueue.isEmpty() || startWaitingForRunning) {
        commandSent = true;
    } else if (requestSpeed != -1) {
        const double requestedInput = requestSpeed;
        const double requested = qBound(0.0, requestedInput, MAX_SPEED);
        requestSpeed = -1;
        if (qAbs(requested - currentSpeed().value()) >= minStepSpeed()) {
            writeFrame(jkfitness10800protocol::speedFrame(requested, units),
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
        requestSpeed = -1;
        requestInclination = -100;
        QSettings profile;
        const bool male = profile.value(QZSettings::sex, QZSettings::default_sex)
                              .toString().compare(QStringLiteral("Male"), Qt::CaseInsensitive) == 0;
        const int age = profile.value(QZSettings::age, QZSettings::default_age).toInt();
        const double heightCm = profile.value(QZSettings::height, QZSettings::default_height).toDouble();
        const double weightKg = profile.value(QZSettings::weight, QZSettings::default_weight).toDouble();
        startSequenceTimer.stop();
        startSequenceQueue.clear();
        startWaitingForRunning = false;
        writeFrame(jkfitness10800protocol::stopFrame(), QStringLiteral("native start reset"));
        startSequenceQueue.enqueue(jkfitness10800protocol::profileFrame(age, male, heightCm, weightKg, units));
        startSequenceQueue.enqueue(QByteArray::fromHex("55150100"));
        startSequenceQueue.enqueue(jkfitness10800protocol::speedFrame(1.0, units));
        startSequenceQueue.enqueue(jkfitness10800protocol::inclineFrame(qMax(0.0, Inclination.value())));
        startSequenceQueue.enqueue(QByteArray::fromHex("55080101"));
        startSequenceQueue.enqueue(jkfitness10800protocol::startFrame());
        startSequenceQueue.enqueue(jkfitness10800protocol::telemetryQueryFrame());
        startSequenceTimer.start(600);
        commandSent = true;
    }

    if (!commandSent && pollCounter++ >= (POLL_INTERVAL_MS / refresh->interval())) {
        pollCounter = 0;
        writeFrame(jkfitness10800protocol::telemetryQueryFrame(), QStringLiteral("telemetry query"));
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
    heartbeatTimer.stop();
    startupSettleTimer.stop();
    startSequenceTimer.stop();
    startSequenceQueue.clear();
    startWaitingForRunning = false;
    socketReadBuffer.clear();
    initDone = false;
    connectedSignalEmitted = false;
    disconnectedSignalEmitted = false;
    runningStateKnown = false;
    runningState = false;
    pausedState = false;
    deviceTypeKnown = false;
    unitsKnown = false;
    protocolDeviceType = 2;
    units = 0;
    pollCounter = 0;
}

bool jkfitness10800treadmill::connected() {
    return socket && socket->state() == QBluetoothSocket::ConnectedState && deviceTypeKnown && unitsKnown &&
           protocolDeviceType == 2 && initDone;
}
