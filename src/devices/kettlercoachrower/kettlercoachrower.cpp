#include "kettlercoachrower.h"

#include "qzsettings.h"
#include "virtualdevices/virtualrower.h"

#include <QRandomGenerator>
#include <QSettings>
#include <QtGlobal>

namespace {
constexpr quint16 SERIAL_PORT_UUID = 0x1101;
constexpr quint16 HANDSHAKE = 0x0001;
constexpr quint16 SESSION_STATE = 0x00d0;
constexpr quint16 DEVICE_MODE = 0x00d1;
constexpr quint16 DEVICE_DETAILS = 0x00d3;
constexpr quint16 ROWING_TELEMETRY = 0x00e0;
constexpr quint16 ACTIVITY_STATE = 0x00c0;
constexpr quint16 HEART_RATE = 0x0008;
constexpr quint8 SUB_READ = 0x01;
constexpr quint8 SUB_ANSWER = 0x03;
constexpr int POLL_INTERVAL_MS = 500;
constexpr int REQUEST_TIMEOUT_MS = 2000;

// Experimental: three independent successful Kinomap captures each start with
// a different 18-byte client challenge and no preceding application-level seed.
// Test whether the Coach S accepts fresh random bytes rather than replayed
// challenges. Do not assume the random-only construction is proven.
QByteArray freshCoachChallenge() {
    QByteArray challenge(18, '\0');
    auto *rng = QRandomGenerator::system();
    for (int i = 0; i < challenge.size(); ++i) {
        challenge[i] = static_cast<char>(rng->generate() & 0xff);
    }
    return challenge;
}

bool isSerialPortService(const QBluetoothServiceInfo &service) {
    return service.serviceUuid() == QBluetoothUuid(SERIAL_PORT_UUID) ||
           service.serviceName().startsWith(QStringLiteral("SerialPort"), Qt::CaseInsensitive) ||
           service.serviceName().startsWith(QStringLiteral("Serial Port"), Qt::CaseInsensitive);
}
} // namespace

kettlercoachrower::kettlercoachrower(bool noWriteResistance, bool noHeartService)
    : noWriteResistance(noWriteResistance), noHeartService(noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &kettlercoachrower::update);
    refresh->start(POLL_INTERVAL_MS);
    requestTimeoutTimer.setSingleShot(true);
    connect(&requestTimeoutTimer, &QTimer::timeout, this, &kettlercoachrower::requestTimeout);
}

kettlercoachrower::~kettlercoachrower() {
    requestTimeoutTimer.stop();
    if (discoveryAgent) discoveryAgent->stop();
    if (socket) {
        socket->disconnect(this);
        socket->close();
        socket->deleteLater();
        socket = nullptr;
    }
}

bool kettlercoachrower::connected() {
    return initDone && socket && socket->state() == QBluetoothSocket::ConnectedState;
}

uint16_t kettlercoachrower::watts() {
    return currentWatts;
}

quint16 kettlercoachrower::be16(const QByteArray &data, int offset) {
    if (offset < 0 || offset + 1 >= data.size()) return 0;
    return (static_cast<quint16>(static_cast<quint8>(data.at(offset))) << 8) |
           static_cast<quint8>(data.at(offset + 1));
}

void kettlercoachrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    emit debug(QStringLiteral("Found Kettler Coach S: ") + device.name());
    discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    discoveryAgent->setRemoteAddress(device.address());
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
            &kettlercoachrower::serviceDiscovered);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this,
            &kettlercoachrower::serviceFinished);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this,
            &kettlercoachrower::serviceFinished);
    discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
}

void kettlercoachrower::serviceDiscovered(const QBluetoothServiceInfo &service) {
    if (service.device().address() == bluetoothDevice.address() && isSerialPortService(service)) {
        serialPortService = service;
        emit debug(QStringLiteral("Coach S serial port service discovered"));
    }
}

void kettlercoachrower::createSocket() {
    if (socket) return;
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(socket, &QBluetoothSocket::readyRead, this, &kettlercoachrower::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, &kettlercoachrower::rfCommConnected);
    connect(socket, &QBluetoothSocket::disconnected, this, &kettlercoachrower::socketDisconnected);
    connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error),
            this, &kettlercoachrower::onSocketErrorOccurred);
#ifdef Q_OS_ANDROID
    // Kinomap's successful Coach S HCI capture shows encrypted RFCOMM.
    // Use a secured socket only for Coach S; other Kettler drivers are unchanged.
    socket->setPreferredSecurityFlags(QBluetooth::Encryption);
#endif
}

void kettlercoachrower::serviceFinished() {
    if (socket) return; // finished/canceled can both fire; connect only once
    createSocket();
    if (serialPortService.isValid()) {
        socket->connectToService(serialPortService);
    } else {
        socket->connectToService(bluetoothDevice.address(), QBluetoothUuid(SERIAL_PORT_UUID));
    }
}

void kettlercoachrower::rfCommConnected() {
    emit debug(QStringLiteral("Kettler Coach S RFCOMM connected"));
    socketReadBuffer.clear();
    requestQueue.clear();
    requestInFlight = false;
    initDone = false;
    signalledConnection = false;
    signalledDisconnection = false;
    awaitingHandshake = true;
    lastStrokeTime = QDateTime();
    lastDistanceTime = QDateTime();
    lastStrokes = 0;
    lastDistanceMeters = 0;
    activeRequest = Request{HANDSHAKE, SUB_READ, freshCoachChallenge()};
    requestInFlight = true;
    const QByteArray bytes = kettlerclassicprotocol::encodeFrameWithEscapedPayloadLength(
        activeRequest.function, activeRequest.operation, activeRequest.payload);
    emit debug(QStringLiteral("Coach S >> handshake ") + bytes.toHex(' '));
    socket->write(bytes);
    requestTimeoutTimer.start(REQUEST_TIMEOUT_MS);
}

void kettlercoachrower::enqueueRead(quint16 function) {
    requestQueue.enqueue(Request{function, SUB_READ, QByteArray()});
}

void kettlercoachrower::sendNextRequest() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState ||
        requestInFlight || requestQueue.isEmpty()) return;
    activeRequest = requestQueue.dequeue();
    requestInFlight = true;
    const QByteArray packet = kettlerclassicprotocol::encodeFrame(activeRequest.function,
                                                                  activeRequest.operation,
                                                                  activeRequest.payload);
    emit debug(QStringLiteral("Coach S >> ") + packet.toHex(' '));
    socket->write(packet);
    requestTimeoutTimer.start(REQUEST_TIMEOUT_MS);
}

void kettlercoachrower::readSocket() {
    if (!socket) return;
    socketReadBuffer.append(socket->readAll());
    const auto frames = kettlerclassicprotocol::consumeFrames(socketReadBuffer);
    for (const auto &frame : frames) {
        emit debug(QStringLiteral("Coach S << function=0x") + QString::number(frame.function, 16) +
                   QStringLiteral(" sub=0x") + QString::number(frame.subFunction, 16) +
                   QStringLiteral(" payload=") + frame.payload.toHex(' '));
        processFrame(frame);
    }
    if (socketReadBuffer.size() > 8192) {
        emit debug(QStringLiteral("Coach S invalid/excessive RX buffer; clearing"));
        socketReadBuffer.clear();
    }
}

void kettlercoachrower::processFrame(const kettlerclassicprotocol::Frame &frame) {
    if (frame.subFunction != SUB_ANSWER) return;
    if (requestInFlight && frame.function == activeRequest.function) {
        requestTimeoutTimer.stop();
        requestInFlight = false;
        if (awaitingHandshake) {
            awaitingHandshake = false;
            initDone = true;
            // Exact initial reads observed in the successful Kinomap session.
            enqueueRead(SESSION_STATE);
            enqueueRead(DEVICE_MODE);
            enqueueRead(SESSION_STATE);
            enqueueRead(DEVICE_MODE);
            sendNextRequest();
            return;
        }
    }
    if (frame.function == ROWING_TELEMETRY) {
        processTelemetry(frame.payload);
        if (!signalledConnection) {
            signalledConnection = true;
            emit connectedAndDiscovered();
        }
    } else if (frame.function == HEART_RATE && frame.payload.size() == 2) {
        const quint16 hr = be16(frame.payload, 0);
        if (hr > 0 && hr < 250) Heart = hr;
    }
    sendNextRequest();
}

void kettlercoachrower::processTelemetry(const QByteArray &payload) {
    if (payload.size() != 24 || be16(payload, 0) != 0x0004) return;
    const QDateTime now = QDateTime::currentDateTime();

    // Candidate offsets grounded in the 2026-11-15 Kinomap HCI capture.
    // The counters at +2 (0..18) and +16 (0..119) were monotonic in that
    // session. Other fields are deliberately not claimed to be watts/HR.
    // Validate units with a console-vs-QZ hardware test before extending.
    const quint16 strokes = be16(payload, 2);
    const quint16 meters = be16(payload, 16);
    if (strokes >= lastStrokes) {
        if (strokes > lastStrokes) {
            if (lastStrokeTime.isValid()) {
                const qint64 elapsedMs = lastStrokeTime.msecsTo(now);
                if (elapsedMs >= 300 && elapsedMs <= 10000) {
                    Cadence = qBound(0.0, (60000.0 * (strokes - lastStrokes)) / elapsedMs, 80.0);
                }
            }
            lastStrokeTime = now;
            lastStrokes = strokes;
        } else if (lastStrokeTime.isValid() && lastStrokeTime.msecsTo(now) > 6000) {
            Cadence = 0;
        }
        StrokesCount = strokes;
    }

    if (meters >= lastDistanceMeters) {
        if (meters > lastDistanceMeters) {
            if (lastDistanceTime.isValid()) {
                const qint64 elapsedMs = lastDistanceTime.msecsTo(now);
                if (elapsedMs >= 300 && elapsedMs <= 10000) {
                    Speed = qBound(0.0, (meters - lastDistanceMeters) * 3600.0 / elapsedMs, 30.0);
                }
            }
            lastDistanceTime = now;
            lastDistanceMeters = meters;
        } else if (lastDistanceTime.isValid() && lastDistanceTime.msecsTo(now) > 6000) {
            Speed = 0;
        }
        Distance = meters / 1000.0;
        Distance1s = meters / 1000.0;
    }
    emit debug(QStringLiteral("Coach S e0 counters strokes=%1 distanceCandidate=%2 fields=%3")
                   .arg(strokes).arg(meters).arg(QString::fromLatin1(payload.toHex(' '))));
}

void kettlercoachrower::update() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState || !initDone) return;
    if (!requestInFlight && requestQueue.isEmpty()) {
        enqueueRead(DEVICE_DETAILS);
        enqueueRead(ROWING_TELEMETRY);
        enqueueRead(ACTIVITY_STATE);
        enqueueRead(HEART_RATE);
    }
    sendNextRequest();

    if (signalledConnection && !virtualCreated && !hasVirtualDevice()) {
        QSettings settings;
        if (settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
            setVirtualDevice(new virtualrower(this, noWriteResistance, noHeartService),
                             VIRTUAL_DEVICE_MODE::PRIMARY);
            virtualCreated = true;
        }
    }
    update_metrics(false, watts());
}

void kettlercoachrower::requestTimeout() {
    if (!requestInFlight) return;
    emit debug(QStringLiteral("Coach S request timeout function 0x") +
               QString::number(activeRequest.function, 16));
    requestInFlight = false;
    if (awaitingHandshake) {
        disconnectAfterError(QStringLiteral("handshake timed out (experimental random challenge)"));
    } else {
        sendNextRequest();
    }
}

void kettlercoachrower::disconnectAfterError(const QString &reason) {
    emit debug(QStringLiteral("Coach S disconnected: ") + reason);
    requestTimeoutTimer.stop();
    requestQueue.clear();
    requestInFlight = false;
    initDone = false;
    awaitingHandshake = false;
    if (socket) socket->close();
    socketDisconnected();
}

void kettlercoachrower::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    disconnectAfterError(QStringLiteral("socket error ") + QString::number(error));
}

void kettlercoachrower::socketDisconnected() {
    requestTimeoutTimer.stop();
    requestQueue.clear();
    requestInFlight = false;
    initDone = false;
    awaitingHandshake = false;
    if (!signalledDisconnection) {
        signalledDisconnection = true;
        emit disconnected();
    }
}
