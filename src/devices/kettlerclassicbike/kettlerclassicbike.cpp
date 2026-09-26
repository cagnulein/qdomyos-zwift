#include "kettlerclassicbike.h"

#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QSettings>
#include <QtMath>

namespace {
constexpr quint16 SERIAL_PORT_UUID = 0x1101;
constexpr int REQUEST_TIMEOUT_MS = 1000;
constexpr int POLL_INTERVAL_MS = 500;

bool isSerialPortService(const QBluetoothServiceInfo &service) {
    const QString name = service.serviceName();
    return service.serviceUuid() == QBluetoothUuid(SERIAL_PORT_UUID) ||
           name.startsWith(QStringLiteral("SerialPort"), Qt::CaseInsensitive) ||
           name.startsWith(QStringLiteral("Serial Port"), Qt::CaseInsensitive);
}

} // namespace

kettlerclassicbike::kettlerclassicbike(bool noWriteResistance, bool noHeartService, bool testResistance,
                                       int8_t bikeResistanceOffset, double bikeResistanceGain)
    : noWriteResistance(noWriteResistance), noHeartService(noHeartService), testResistance(testResistance),
      bikeResistanceOffset(bikeResistanceOffset), bikeResistanceGain(bikeResistanceGain) {
    Q_UNUSED(testResistance);
    Q_UNUSED(bikeResistanceOffset);
    Q_UNUSED(bikeResistanceGain);

    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &kettlerclassicbike::update);
    refresh->start(POLL_INTERVAL_MS);
    connect(&requestTimeoutTimer, &QTimer::timeout, this, &kettlerclassicbike::requestTimeout);
}

kettlerclassicbike::~kettlerclassicbike() {
    requestTimeoutTimer.stop();
    if (socket) {
        socket->disconnect(this);
        socket->close();
        socket->deleteLater();
        socket = nullptr;
    }
    if (discoveryAgent) {
        discoveryAgent->stop();
    }
}

void kettlerclassicbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    emit debug(QStringLiteral("Found Kettler Classic device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
            &kettlerclassicbike::serviceDiscovered);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, &kettlerclassicbike::serviceFinished);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this, &kettlerclassicbike::serviceFinished);
    discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
}

void kettlerclassicbike::serviceDiscovered(const QBluetoothServiceInfo &service) {
    if (service.device().address() != bluetoothDevice.address() || !isSerialPortService(service)) {
        return;
    }

    serialPortService = service;
    emit debug(QStringLiteral("Kettler Classic Serial Port service: ") + service.serviceUuid().toString());
}

void kettlerclassicbike::createSocket() {
    if (socket) {
        return;
    }

    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(socket, &QBluetoothSocket::readyRead, this, &kettlerclassicbike::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&kettlerclassicbike::rfCommConnected));
    connect(socket, &QBluetoothSocket::disconnected, this, &kettlerclassicbike::socketDisconnected);
    connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this,
            &kettlerclassicbike::onSocketErrorOccurred);
#ifdef Q_OS_ANDROID
    socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif
}

void kettlerclassicbike::serviceFinished() {
    createSocket();
    if (serialPortService.isValid()) {
        emit debug(QStringLiteral("Connecting to Kettler Classic Serial Port service"));
        socket->connectToService(serialPortService);
    } else {
        emit debug(QStringLiteral("Serial Port service was not reported; trying UUID 1101 directly"));
        socket->connectToService(bluetoothDevice.address(), QBluetoothUuid(SERIAL_PORT_UUID));
    }
}

void kettlerclassicbike::rfCommConnected() {
    emit debug(QStringLiteral("Kettler Classic RFCOMM connected: ") + socket->peerName());
    requestQueue.clear();
    requestInFlight = false;
    initDone = false;
    setupQueued = true;
    connectedSignalEmitted = false;
    disconnectedSignalEmitted = false;
    socketReadBuffer.clear();
    lastTelemetryUpdate = QDateTime::currentDateTime();
    enqueueSetupRequests();
    sendNextRequest();
}

void kettlerclassicbike::enqueueRequest(quint16 function, quint8 subFunction, const QByteArray &payload) {
    Request request;
    request.function = function;
    request.subFunction = subFunction;
    request.payload = payload;
    request.expectsResponse = subFunction != SUB_WRITE && subFunction != SUB_RESET;
    requestQueue.enqueue(request);
}

void kettlerclassicbike::enqueueRead(quint16 function) {
    enqueueRequest(function, SUB_READ);
}

QByteArray kettlerclassicbike::uint16Bytes(quint16 value) {
    return QByteArray{static_cast<char>((value >> 8) & 0xff), static_cast<char>(value & 0xff)};
}

quint32 kettlerclassicbike::readBigEndian(const QByteArray &payload) {
    if (payload.isEmpty() || payload.size() > 4) {
        return 0;
    }
    quint32 value = 0;
    for (const char byte : payload) {
        value = (value << 8) | static_cast<quint8>(byte);
    }
    return value;
}

void kettlerclassicbike::enqueueSetupRequests() {
    requestQueue.clear();
    enqueueRequest(HANDSHAKE, SUB_READ,
                   QByteArray::fromHex("2d24f22496a4ff9829f921be9daa9e4d0117"));
    enqueueRead(FIRMWARE_VERSION);
    enqueueRead(DEVICE_TYPE);
    enqueueRead(DEVICE_NAME);
    enqueueRead(DEVICE_STATE);
    enqueueRead(DEVICE_IDENTIFIER);
    enqueueRead(SERIAL_NUMBER);
    enqueueRead(ARTICLE_NUMBER);
    enqueueRead(CALIBRATION_VERSION);
    enqueueRequest(DEVICE_BRAKE_MODE, SUB_WRITE, uint16Bytes(1));
    enqueueRead(POWER_MIN);
    enqueueRead(POWER_MAX);
    enqueueRead(DEVICE_BRAKE_MODE);
    enqueueRead(BRAKE_LEVEL_MIN);
    enqueueRead(BRAKE_LEVEL_MAX);
}

void kettlerclassicbike::enqueueTelemetryRequests() {
    enqueueRead(RPM);
    enqueueRead(POWER_TARGET);
    enqueueRead(POWER_CURRENT);
    enqueueRead(BRAKE_LEVEL);
    enqueueRead(PULSE);
}

void kettlerclassicbike::enqueueResistanceRequest(resistance_t resistance) {
    if (noWriteResistance) {
        return;
    }

    const quint8 clamped = static_cast<quint8>(qBound<int>(minLevel, resistance, maxLevel));
    enqueueRequest(BRAKE_LEVEL, SUB_WRITE, QByteArray(1, static_cast<char>(clamped)));
    enqueueRead(BRAKE_LEVEL);
}

void kettlerclassicbike::enqueueWorkoutState(bool paused) {
    enqueueRequest(START_STOP, SUB_WRITE, QByteArray(1, static_cast<char>(paused ? 0 : 1)));
}

void kettlerclassicbike::sendNextRequest() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState || requestInFlight || requestQueue.isEmpty()) {
        return;
    }

    activeRequest = requestQueue.dequeue();
    requestInFlight = true;
    const QByteArray packet = kettlerclassicprotocol::encodeFrame(activeRequest.function, activeRequest.subFunction,
                                                                  activeRequest.payload);
    emit debug(QStringLiteral(">> ") + packet.toHex(' '));
    socket->write(packet);

    if (activeRequest.expectsResponse) {
        requestTimeoutTimer.start(REQUEST_TIMEOUT_MS);
    } else {
        requestInFlight = false;
        QTimer::singleShot(0, this, &kettlerclassicbike::sendNextRequest);
    }
}

void kettlerclassicbike::requestTimeout() {
    if (!requestInFlight) {
        return;
    }
    emit debug(QStringLiteral("Kettler Classic request timeout for function 0x") +
               QString::number(activeRequest.function, 16));
    requestInFlight = false;
    if (setupQueued) {
        abortSetup(QStringLiteral("setup request timed out"));
        return;
    }
    sendNextRequest();
}

void kettlerclassicbike::abortSetup(const QString &reason) {
    emit debug(QStringLiteral("Kettler Classic setup failed: ") + reason);
    requestTimeoutTimer.stop();
    requestQueue.clear();
    requestInFlight = false;
    setupQueued = false;
    initDone = false;
    if (socket) {
        socket->close();
    }
    socketDisconnected();
}

void kettlerclassicbike::readSocket() {
    if (!socket) {
        return;
    }

    socketReadBuffer.append(socket->readAll());
    const QVector<kettlerclassicprotocol::Frame> frames = kettlerclassicprotocol::consumeFrames(socketReadBuffer);
    for (const kettlerclassicprotocol::Frame &frame : frames) {
        emit debug(QStringLiteral("<< function=0x") + QString::number(frame.function, 16) +
                   QStringLiteral(" sub=0x") + QString::number(frame.subFunction, 16) +
                   QStringLiteral(" payload=") + frame.payload.toHex(' '));
        processFrame(frame);
    }
}

void kettlerclassicbike::processFrame(const kettlerclassicprotocol::Frame &frame) {
    if (frame.subFunction == SUB_ANSWER) {
        handleAnswer(frame);
    }

    if (requestInFlight && activeRequest.function == frame.function && frame.subFunction == SUB_ERROR) {
        requestTimeoutTimer.stop();
        requestInFlight = false;
        if (setupQueued) {
            abortSetup(QStringLiteral("device returned an error for function 0x") +
                       QString::number(frame.function, 16));
            return;
        }
        sendNextRequest();
    } else if (requestInFlight && activeRequest.function == frame.function &&
               (frame.subFunction == SUB_ANSWER || frame.subFunction == SUB_STATUS)) {
        requestTimeoutTimer.stop();
        requestInFlight = false;
        sendNextRequest();
    }
}

void kettlerclassicbike::handleAnswer(const kettlerclassicprotocol::Frame &frame) {
    switch (frame.function) {
    case POWER_MIN:
        if (frame.payload.size() >= 2) minPower = static_cast<quint16>(readBigEndian(frame.payload));
        break;
    case POWER_MAX:
        if (frame.payload.size() >= 2) maxPower = static_cast<quint16>(readBigEndian(frame.payload));
        break;
    case BRAKE_LEVEL_MIN:
        if (!frame.payload.isEmpty()) minLevel = static_cast<quint8>(readBigEndian(frame.payload));
        break;
    case BRAKE_LEVEL_MAX:
        if (!frame.payload.isEmpty()) maxLevel = static_cast<quint8>(readBigEndian(frame.payload));
        break;
    case RPM:
        if (frame.payload.size() >= 2) rpm = static_cast<quint16>(readBigEndian(frame.payload));
        break;
    case POWER_CURRENT:
        if (frame.payload.size() >= 2) power = static_cast<quint16>(readBigEndian(frame.payload));
        break;
    case PULSE:
        if (frame.payload.size() >= 2) pulse = static_cast<quint16>(readBigEndian(frame.payload));
        break;
    case BRAKE_LEVEL:
        if (!frame.payload.isEmpty()) {
            level = static_cast<quint8>(readBigEndian(frame.payload));
            Resistance = level;
        }
        break;
    default:
        break;
    }

    if (frame.function == RPM || frame.function == PULSE) {
        updateMetricsFromCadence();
    }
    Heart = pulse;
    Cadence = rpm;
    m_watt = power;
}

void kettlerclassicbike::finishSetup() {
    if (initDone || !setupQueued || requestInFlight || !requestQueue.isEmpty()) {
        return;
    }

    setupQueued = false;
    initDone = true;
    if (!connectedSignalEmitted) {
        connectedSignalEmitted = true;
        emit connectedAndDiscovered();
    }
}

void kettlerclassicbike::updateMetricsFromCadence() {
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 elapsedMs = lastTelemetryUpdate.msecsTo(now);
    if (elapsedMs > 0 && elapsedMs < 10000 && rpm > 0) {
        const double speed = (static_cast<double>(rpm) * static_cast<double>(diameterInMM) * M_PI * 60.0) / 1000000.0;
        Speed = speed;
        Distance += speed * (static_cast<double>(elapsedMs) / 3600000.0);
        KCal += (0.38 * speed * (static_cast<double>(elapsedMs) / 1000.0)) / 60.0;
    } else if (rpm == 0) {
        Speed = 0;
    }
    lastTelemetryUpdate = now;
}

void kettlerclassicbike::update() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState) {
        return;
    }

    if (!initDone) {
        finishSetup();
        return;
    }

    if (requestResistance != -1) {
        const resistance_t requested = requestResistance;
        requestResistance = -1;
        enqueueResistanceRequest(requested);
    }

    if (!requestInFlight && requestQueue.isEmpty()) {
        enqueueTelemetryRequests();
    }
    sendNextRequest();

    if (!firstStateChanged && !hasVirtualDevice()) {
        QSettings settings;
        if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset,
                                               bikeResistanceGain);
            connect(virtualBike, &virtualbike::changeInclination, this, &kettlerclassicbike::changeInclination);
            setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
        firstStateChanged = true;
    }

    update_metrics(false, watts());
}

bool kettlerclassicbike::connected() {
    return socket && socket->state() == QBluetoothSocket::ConnectedState && initDone;
}

resistance_t kettlerclassicbike::maxResistance() {
    return maxLevel > 0 ? maxLevel : 32;
}

uint16_t kettlerclassicbike::watts() {
    return power;
}

void kettlerclassicbike::setPaused(bool p) {
    bike::setPaused(p);
    if (socket && socket->state() == QBluetoothSocket::ConnectedState && initDone) {
        enqueueWorkoutState(p);
        sendNextRequest();
    }
}

void kettlerclassicbike::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    emit debug(QStringLiteral("Kettler Classic RFCOMM error: ") + QString::number(error));
    if (setupQueued) {
        abortSetup(QStringLiteral("socket error"));
    } else {
        socketDisconnected();
    }
}

void kettlerclassicbike::socketDisconnected() {
    requestTimeoutTimer.stop();
    requestQueue.clear();
    requestInFlight = false;
    initDone = false;
    setupQueued = false;
    if (!disconnectedSignalEmitted) {
        disconnectedSignalEmitted = true;
        emit disconnected();
    }
}
