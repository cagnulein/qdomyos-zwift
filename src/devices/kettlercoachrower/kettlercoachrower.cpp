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
// Optional Kettler S-FIT IRMP commands; Coach S support is hardware-dependent.
constexpr quint16 POWER_CURRENT = 0x000b;
constexpr quint16 BRAKE_LEVEL = 0x001b;
constexpr quint16 BRAKE_LEVEL_MIN = 0x001c;
constexpr quint16 BRAKE_LEVEL_MAX = 0x001d;
constexpr quint8 SUB_READ = 0x01;
constexpr quint8 SUB_WRITE = 0x02;
constexpr quint8 SUB_ANSWER = 0x03;
constexpr quint8 SUB_STATUS = 0x04;
constexpr quint8 SUB_ERROR = 0x05;
constexpr int POLL_INTERVAL_MS = 500;
constexpr int REQUEST_TIMEOUT_MS = 2000;

// Kinomap / Kettler S-FIT authentication: the client supplies two random
// salt bytes and an MD5 digest of salt || fixed protocol key. The rower
// responds with another salt/digest pair, verified before starting telemetry.
QByteArray freshCoachChallenge() {
    const quint32 randomValue = QRandomGenerator::system()->generate();
    QByteArray salt;
    salt.append(static_cast<char>(randomValue & 0xff));
    salt.append(static_cast<char>((randomValue >> 8) & 0xff));
    return kettlerclassicprotocol::coachSHandshakeForSalt(salt);
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

resistance_t kettlercoachrower::maxResistance() {
    return minLevelReceived && maxLevelReceived && maxBrakeLevel >= minBrakeLevel
               ? static_cast<resistance_t>(maxBrakeLevel)
               : 30; // S-FIT default only until the Coach S reports actual limits
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
    if (service.device().address() == bluetoothDevice.address() &&
        service.serviceUuid() == QBluetoothUuid(SERIAL_PORT_UUID)) {
        emit debug(QStringLiteral("Coach S SPP service discovered (UUID 0x1101)"));
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
    // Kinomap uses the standard Serial Port Profile UUID (0x1101).
    // Never connect using a discovered service selected by its name: the
    // Coach S also exposes a proprietary SerialPort-named service, and QZ
    // previously picked that UUID despite SPP being advertised.
    const QBluetoothUuid sppUuid(SERIAL_PORT_UUID);
    emit debug(QStringLiteral("Coach S connecting explicitly to SPP UUID ") + sppUuid.toString());
    socket->connectToService(bluetoothDevice.address(), sppUuid);
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
    telemetrySeen = false;
    powerReadEnabled = true;
    brakeReadEnabled = true;
    minLevelReceived = false;
    maxLevelReceived = false;
    minBrakeLevel = 1;
    maxBrakeLevel = 30;
    optionalProbeStage = 0;
    telemetryPollBatch = 0;
    requestResistance = -1;
    currentWatts = 0;
    activeRequest = Request{HANDSHAKE, SUB_READ, freshCoachChallenge()};
    requestInFlight = true;
    const QByteArray bytes = kettlerclassicprotocol::encodeFrameWithEscapedPayloadLength(
        activeRequest.function, activeRequest.operation, activeRequest.payload);
    emit debug(QStringLiteral("Coach S >> handshake ") + bytes.toHex(' '));
    const qint64 queued = socket->write(bytes);
    emit debug(QStringLiteral("Coach S handshake socket write queued=%1 expected=%2")
                   .arg(queued).arg(bytes.size()));
    if (queued != bytes.size()) {
        disconnectAfterError(QStringLiteral("handshake socket write failed or was partial"));
        return;
    }
    requestTimeoutTimer.start(REQUEST_TIMEOUT_MS);
}

void kettlercoachrower::enqueueRead(quint16 function) {
    requestQueue.enqueue(Request{function, SUB_READ, QByteArray(), true});
}

void kettlercoachrower::enqueueWrite(quint16 function, const QByteArray &payload) {
    // S-FIT does not guarantee a correlated ACK for writes. Check the level
    // with a separate READ and keep telemetry running if no write reply arrives.
    requestQueue.enqueue(Request{function, SUB_WRITE, payload, false});
}

void kettlercoachrower::enqueueResistanceRequest(resistance_t requested) {
    if (noWriteResistance) {
        emit debug(QStringLiteral("Coach S resistance write disabled by QZ setting"));
        return;
    }
    const bool validLimits = minLevelReceived && maxLevelReceived &&
                             maxBrakeLevel >= minBrakeLevel;
    const int minLevel = validLimits ? minBrakeLevel : 1;
    const int maxLevel = validLimits ? maxBrakeLevel : 30;
    const quint8 level = static_cast<quint8>(qBound(minLevel, static_cast<int>(requested), maxLevel));
    emit debug(QStringLiteral("Coach S experimental brake write: requested=%1 level=%2 range=%3..%4")
                   .arg(requested).arg(level).arg(minLevel).arg(maxLevel));
    enqueueWrite(BRAKE_LEVEL, QByteArray(1, static_cast<char>(level)));
    enqueueRead(BRAKE_LEVEL); // read back what the firmware actually reports
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
    const qint64 queued = socket->write(packet);
    emit debug(QStringLiteral("Coach S socket write queued=%1 expected=%2")
                   .arg(queued).arg(packet.size()));
    if (queued != packet.size()) {
        emit debug(QStringLiteral("Coach S failed to queue IRMP request function=0x") +
                   QString::number(activeRequest.function, 16));
        requestInFlight = false;
        QTimer::singleShot(0, this, &kettlercoachrower::sendNextRequest);
        return;
    }
    if (activeRequest.expectsResponse) {
        requestTimeoutTimer.start(REQUEST_TIMEOUT_MS);
    } else {
        requestInFlight = false;
        QTimer::singleShot(0, this, &kettlercoachrower::sendNextRequest);
    }
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
    // A new Coach S may reject S-FIT optional commands. Never break the
    // proven 0x00E0 telemetry stream because a probe is unsupported.
    if (frame.subFunction == SUB_ERROR) {
        emit debug(QStringLiteral("Coach S IRMP error for function=0x") +
                   QString::number(frame.function, 16) +
                   QStringLiteral(" payload=") + frame.payload.toHex(' '));
        if (requestInFlight && frame.function == activeRequest.function) {
            requestTimeoutTimer.stop();
            requestInFlight = false;
            if (frame.function == POWER_CURRENT) powerReadEnabled = false;
            if (frame.function == BRAKE_LEVEL && activeRequest.operation == SUB_READ)
                brakeReadEnabled = false;
            if (awaitingHandshake) {
                disconnectAfterError(QStringLiteral("handshake rejected by Coach S"));
                return;
            }
            sendNextRequest();
        }
        return;
    }
    if (frame.subFunction != SUB_ANSWER && frame.subFunction != SUB_STATUS) return;

    if (frame.subFunction == SUB_ANSWER) {
        switch (frame.function) {
        case POWER_CURRENT:
            if (frame.payload.size() == 2) {
                currentWatts = be16(frame.payload, 0);
                m_watt = currentWatts;
                emit debug(QStringLiteral("Coach S IRMP power current=%1 W").arg(currentWatts));
            }
            break;
        case BRAKE_LEVEL:
            if (frame.payload.size() == 1) {
                const quint8 level = static_cast<quint8>(frame.payload.at(0));
                Resistance = level;
                emit resistanceRead(level);
                emit debug(QStringLiteral("Coach S IRMP brake level=%1").arg(level));
            }
            break;
        case BRAKE_LEVEL_MIN:
            if (frame.payload.size() == 1) {
                minBrakeLevel = static_cast<quint8>(frame.payload.at(0));
                minLevelReceived = true;
                emit debug(QStringLiteral("Coach S IRMP brake min=%1").arg(minBrakeLevel));
            }
            break;
        case BRAKE_LEVEL_MAX:
            if (frame.payload.size() == 1) {
                maxBrakeLevel = static_cast<quint8>(frame.payload.at(0));
                maxLevelReceived = true;
                emit debug(QStringLiteral("Coach S IRMP brake max=%1").arg(maxBrakeLevel));
            }
            break;
        default:
            break;
        }
    }

    if (requestInFlight && frame.function == activeRequest.function) {
        if (awaitingHandshake && !kettlerclassicprotocol::isValidCoachSHandshake(frame.payload)) {
            disconnectAfterError(QStringLiteral("invalid Coach S handshake response digest"));
            return;
        }
        requestTimeoutTimer.stop();
        requestInFlight = false;
        if (awaitingHandshake) {
            awaitingHandshake = false;
            initDone = true;
            // Keep the exact working Kinomap setup before optional probes.
            enqueueRead(SESSION_STATE);
            enqueueRead(DEVICE_MODE);
            enqueueRead(SESSION_STATE);
            enqueueRead(DEVICE_MODE);
            sendNextRequest();
            return;
        }
    }
    if (frame.function == ROWING_TELEMETRY && frame.subFunction == SUB_ANSWER) {
        processTelemetry(frame.payload);
        telemetrySeen = true;
        if (!signalledConnection) {
            signalledConnection = true;
            emit connectedAndDiscovered();
        }
    } else if (frame.function == HEART_RATE && frame.payload.size() == 2 &&
               frame.subFunction == SUB_ANSWER) {
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

    // Only send a brake write in response to an explicit QZ resistance
    // request. Never switch brake mode (0x0007) automatically.
    if (telemetrySeen && requestResistance != -1) {
        const resistance_t requested = requestResistance;
        requestResistance = -1;
        enqueueResistanceRequest(requested);
    }

    if (!requestInFlight && requestQueue.isEmpty()) {
        // Maintain the working Coach S polling sequence at all times.
        enqueueRead(DEVICE_DETAILS);
        enqueueRead(ROWING_TELEMETRY);
        enqueueRead(ACTIVITY_STATE);
        enqueueRead(HEART_RATE);

        if (telemetrySeen) {
            // Probe each optional command once, separated by normal polling
            // batches. If unsupported, 2-second timeouts won't repeat.
            switch (optionalProbeStage) {
            case 0: enqueueRead(BRAKE_LEVEL_MIN); break;
            case 1: enqueueRead(BRAKE_LEVEL_MAX); break;
            case 2: enqueueRead(POWER_CURRENT); break;
            case 3: enqueueRead(BRAKE_LEVEL); break;
            default:
                if (powerReadEnabled) enqueueRead(POWER_CURRENT);
                if (brakeReadEnabled && (telemetryPollBatch % 3 == 0))
                    enqueueRead(BRAKE_LEVEL);
                break;
            }
            if (optionalProbeStage < 4) ++optionalProbeStage;
            ++telemetryPollBatch;
        }
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
               QString::number(activeRequest.function, 16) +
               QStringLiteral(" operation=0x") + QString::number(activeRequest.operation, 16));
    if (activeRequest.function == POWER_CURRENT) powerReadEnabled = false;
    if (activeRequest.function == BRAKE_LEVEL && activeRequest.operation == SUB_READ)
        brakeReadEnabled = false;
    requestInFlight = false;
    if (awaitingHandshake) {
        disconnectAfterError(QStringLiteral("handshake timed out (Coach S MD5 salt challenge)"));
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
