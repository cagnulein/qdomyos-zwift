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
// Confirmed from Kinomap HCI and Coach S physical-load test: C0/02 sets
// resistance level (1..10), C0/03 reports the active level.
constexpr quint16 RESISTANCE_LEVEL = 0x00c0;
constexpr quint16 HEART_RATE = 0x0008;
constexpr quint8 SUB_READ = 0x01;
constexpr quint8 SUB_WRITE = 0x02;
constexpr quint8 SUB_ANSWER = 0x03;
constexpr quint8 SUB_STATUS = 0x04;
constexpr quint8 SUB_ERROR = 0x05;
constexpr int POLL_INTERVAL_MS = 500;
constexpr int REQUEST_TIMEOUT_MS = 2000;
// Jacek's Coach S console uses L4 as the starting physical load.
constexpr int DEFAULT_COACH_S_RESISTANCE = 4;

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
    return 10; // Coach S manual: ten user-selectable resistance levels.
}

void kettlercoachrower::changeResistance(resistance_t resistance) {
    rower::changeResistance(resistance);
    if (!noWriteResistance && autoResistance() && requestResistance != -1) {
        // The Coach S ignores the S-FIT brake READ (0x001B/01), so QZ's +/- buttons
        // must use the last requested level rather than repeatedly starting at 0.
        // This is a requested TARGET, not verified mechanical resistance.
        const int target = qBound(1, static_cast<int>(requestResistance),
                                  static_cast<int>(maxResistance()));
        Resistance = target;
        emit debug(QStringLiteral("Coach S provisional brake target=%1 (not confirmed by device)")
                       .arg(target));
    }
}

quint16 kettlercoachrower::be16(const QByteArray &data, int offset) {
    if (offset < 0 || offset + 1 >= data.size()) return 0;
    return (static_cast<quint16>(static_cast<quint8>(data.at(offset))) << 8) |
           static_cast<quint8>(data.at(offset + 1));
}

#ifdef Q_OS_ANDROID
void kettlercoachrower::connectToBondedDevice(const QBluetoothDeviceInfo &device) {
    bluetoothDevice = device;
    emit debug(QStringLiteral("Coach S connecting directly to bonded SPP address ") +
               device.address().toString());
    // The existing secured RFCOMM socket performs the same successful
    // S-FIT handshake in rfCommConnected(); only the inquiry/SDP scan is skipped.
    createSocket();
    socket->connectToService(device.address(), QBluetoothUuid(SERIAL_PORT_UUID));
}
#endif

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
    sessionCounters.reset();
    powerBasedPace.reset();
    lastPowerBasedMode = false;
    rawDistanceOffsetMeters = 0.0;
    powerClock.start();
    resetStrokeMetrics();
    Distance = 0;
    Distance1s = 0;
    StrokesCount = 0;
    telemetrySeen = false;
    requestResistance = -1;
    pendingResistanceLevel = -1;
    lastReportedResistanceLevel = -1;
    Resistance = 0;
    RequestedResistance = 0;
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
    // S-FIT does not guarantee a correlated ACK for writes; never block telemetry.
    requestQueue.enqueue(Request{function, SUB_WRITE, payload, false});
}

void kettlercoachrower::enqueueResistanceRequest(resistance_t requested) {
    if (noWriteResistance) {
        emit debug(QStringLiteral("Coach S resistance write disabled by QZ setting"));
        return;
    }
    const int level = qBound(1, static_cast<int>(requested),
                             static_cast<int>(maxResistance()));
    pendingResistanceLevel = level;
    // Confirmed by an HCI capture and by the user's physical-load changes
    // L4 -> L6 -> L5 -> L3 -> L4. Only send the single level byte.
    emit debug(QStringLiteral("Coach S resistance C0/02 write requested level=%1").arg(level));
    enqueueWrite(RESISTANCE_LEVEL, QByteArray(1, static_cast<char>(level)));
    // Do not reintroduce unsupported S-FIT reads: they stall telemetry.
}

void kettlercoachrower::sendNextRequest() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState ||
        requestInFlight || requestQueue.isEmpty()) return;
    activeRequest = requestQueue.dequeue();
    requestInFlight = true;
    // Kinomap counts escaped payload bytes in the advertised length of
    // Coach S C0/02 writes. For L2/L3, encoded payload has two bytes.
    // Other devices and ordinary reads keep the standard frame encoder.
    const QByteArray packet =
        (activeRequest.function == RESISTANCE_LEVEL &&
         activeRequest.operation == SUB_WRITE)
            ? kettlerclassicprotocol::encodeFrameWithEscapedPayloadLength(
                  activeRequest.function, activeRequest.operation, activeRequest.payload)
            : kettlerclassicprotocol::encodeFrame(
                  activeRequest.function, activeRequest.operation, activeRequest.payload);
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
    if (frame.subFunction == SUB_ERROR) {
        emit debug(QStringLiteral("Coach S IRMP error for function=0x") +
                   QString::number(frame.function, 16) +
                   QStringLiteral(" payload=") + frame.payload.toHex(' '));
        if (requestInFlight && frame.function == activeRequest.function) {
            requestTimeoutTimer.stop();
            requestInFlight = false;
            if (awaitingHandshake) {
                disconnectAfterError(QStringLiteral("handshake rejected by Coach S"));
                return;
            }
            sendNextRequest();
        }
        return;
    }
    if (frame.subFunction != SUB_ANSWER && frame.subFunction != SUB_STATUS) return;

    if (frame.function == RESISTANCE_LEVEL && frame.subFunction == SUB_ANSWER &&
        frame.payload.size() == 1) {
        recordReportedResistance(static_cast<quint8>(frame.payload.at(0)), "C0/03");
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
            // Keep the exact working Kinomap setup before regular telemetry.
            enqueueRead(SESSION_STATE);
            enqueueRead(DEVICE_MODE);
            enqueueRead(SESSION_STATE);
            enqueueRead(DEVICE_MODE);
            // Apply the console's practical L4 before the first stroke.
            // Keep the known-good post-auth setup reads ahead of the write.
            // Explicit "No write resistance" still takes precedence.
            if (!noWriteResistance) {
                emit debug(QStringLiteral("Coach S setting default resistance L4 after authentication"));
                enqueueResistanceRequest(DEFAULT_COACH_S_RESISTANCE);
            }
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

void kettlercoachrower::resetStrokeMetrics() {
    strokePower.reset();
    cadenceSmoother.reset();
    lastStrokeMs = -1;
    lastStrokes = sessionCounters.rawStrokes();
    lastStrokeDistanceMeters = sessionCounters.rawMeters();
    currentWatts = 0;
    m_watt = 0;
    Speed = 0;
    Cadence = 0;
    if (sessionCounters.hasSample() && powerClock.isValid())
        strokePower.addSample(lastStrokes, 0, powerClock.elapsed());
}

void kettlercoachrower::clearStats() {
    rower::clearStats();
    // Start a new QZ workout at the latest physical counter values.
    sessionCounters.clearWorkout();
    powerBasedPace.reset();
    lastPowerBasedMode = false;
    rawDistanceOffsetMeters = 0.0;
    resetStrokeMetrics();
    Distance = 0;
    Distance1s = 0;
    StrokesCount = 0;
}

void kettlercoachrower::updateStrokeIdle(qint64 nowMs) {
    if (strokePower.active(nowMs)) return;
    cadenceSmoother.reset();
    if (currentWatts != 0 || Speed.value() != 0 || Cadence.value() != 0) {
        currentWatts = 0;
        m_watt = 0;
        Speed = 0;
        Cadence = 0;
        emit debug(QStringLiteral("Coach S no completed stroke: power, speed and cadence idle"));
    }
}

void kettlercoachrower::processTelemetry(const QByteArray &payload) {
    if (payload.size() != 24 || be16(payload, 0) != 0x0004) return;
    const qint64 nowMs = powerClock.elapsed();
    const quint16 strokes = be16(payload, 2);
    const quint16 meters = be16(payload, 16);
    const quint16 rawWatts = kettlerclassicprotocol::coachSCandidateWatts(payload);
    const quint16 consoleStrokeWatts = kettlerclassicprotocol::coachSStrokeWatts(payload);
    const bool precedingStrokeActive = strokePower.active(nowMs);
    QSettings settings;
    const bool powerBasedMode = settings.value(
        QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool();
    const bool completedStroke = sessionCounters.hasSample() &&
                                 strokes > sessionCounters.rawStrokes();
    const bool strokeActive = strokePower.active(nowMs) || completedStroke;

    // The physical console's distance can advance without new strokes.
    // A stroke boundary commits the distance, whereas an idle sample rebases
    // the raw counter. Keep the validated raw-counter pace path unchanged.
    if (sessionCounters.addSample(strokes, meters, strokeActive))
        resetStrokeMetrics();

    updateStrokeIdle(nowMs);
    currentWatts = strokePower.addSample(strokes, consoleStrokeWatts, nowMs);
    m_watt = currentWatts;
    recordReportedResistance(kettlerclassicprotocol::coachSReportedResistance(payload),
                             "E0 byte 23");

    if (powerBasedMode != lastPowerBasedMode) {
        // Preserve the displayed session distance if the setting is toggled
        // while QZ is running, without changing raw console counters.
        const double displayedMeters = Distance.value() * 1000.0;
        if (powerBasedMode)
            powerBasedPace.reset(displayedMeters);
        else
            rawDistanceOffsetMeters = displayedMeters -
                                      static_cast<double>(sessionCounters.meters());
        lastPowerBasedMode = powerBasedMode;
        emit debug(QStringLiteral("Coach S Concept2 power-based pace %1")
                       .arg(powerBasedMode ? QStringLiteral("enabled")
                                           : QStringLiteral("disabled")));
    }

    if (strokes > lastStrokes) {
        // Smooth only Coach S SPM; preserve watts, speed, distance and PM5.
        Cadence = cadenceSmoother.completedStroke(strokes, nowMs, precedingStrokeActive);
        if (powerBasedMode) {
            const qint64 elapsedMs = lastStrokeMs >= 0 ? nowMs - lastStrokeMs : -1;
            powerBasedPace.completedStroke(currentWatts, elapsedMs,
                                           precedingStrokeActive);
            Speed = powerBasedPace.speedKmh();
            emit debug(QStringLiteral("Coach S Concept2-equivalent speed=%1 km/h power=%2 W sessionMeters=%3")
                           .arg(Speed.value(), 0, 'f', 2).arg(currentWatts)
                           .arg(powerBasedPace.meters(), 0, 'f', 2));
        }
        if (lastStrokeMs >= 0) {
            const qint64 elapsedMs = nowMs - lastStrokeMs;
            if (elapsedMs >= 300 && elapsedMs <= 6500) {
                const quint16 strokesDelta = strokes - lastStrokes;
                if (!powerBasedMode && meters > lastStrokeDistanceMeters) {
                    const int distanceDelta = meters - lastStrokeDistanceMeters;
                    const double strokeSpeedKmh =
                        qBound(0.0, distanceDelta * 3600.0 / elapsedMs, 30.0);
                    // Preserve the existing, hardware-validated pace EMA.
                    const double previousSpeedKmh = Speed.value();
                    const double displaySpeedKmh = previousSpeedKmh > 0.0
                        ? 0.65 * previousSpeedKmh + 0.35 * strokeSpeedKmh
                        : strokeSpeedKmh;
                    Speed = displaySpeedKmh;
                    emit debug(QStringLiteral(
                        "Coach S completed-stroke speed=%1 km/h rawStrokeSpeed=%2 km/h distanceDelta=%3 m elapsedMs=%4 strokesDelta=%5")
                                   .arg(displaySpeedKmh, 0, 'f', 2)
                                   .arg(strokeSpeedKmh, 0, 'f', 2)
                                   .arg(distanceDelta).arg(elapsedMs)
                                   .arg(strokesDelta));
                }
            }
        }
        lastStrokeMs = nowMs;
        lastStrokeDistanceMeters = meters;
        lastStrokes = strokes;
    }

    StrokesCount = static_cast<double>(sessionCounters.strokes());
    const double displayedMeters = powerBasedMode
        ? powerBasedPace.meters()
        : qMax(0.0, static_cast<double>(sessionCounters.meters()) +
                        rawDistanceOffsetMeters);
    Distance = displayedMeters / 1000.0;
    Distance1s = displayedMeters / 1000.0;
    emit debug(QStringLiteral("Coach S e0 counters strokes=%1 distanceCandidate=%2 powerCandidate=%3 smoothedWatts=%4 fields=%5 sessionStrokes=%6 sessionMeters=%7 consoleStrokeWatts=%8")
                   .arg(strokes).arg(meters).arg(rawWatts).arg(currentWatts)
                   .arg(QString::fromLatin1(payload.toHex(' ')))
                   .arg(sessionCounters.strokes()).arg(sessionCounters.meters())
                   .arg(consoleStrokeWatts));
}

void kettlercoachrower::recordReportedResistance(quint8 level, const char *source) {
    if (level < 1 || level > maxResistance()) return;

    const bool changed = lastReportedResistanceLevel != level;
    lastReportedResistanceLevel = level;
    if (pendingResistanceLevel != -1 && level != pendingResistanceLevel) {
        // The prior value may still be in flight. Preserve the requested
        // target for rapid +/- presses until the device reports the new one.
        if (changed) {
            emit debug(QStringLiteral("Coach S resistance observed level=%1 via %2; awaiting level=%3")
                           .arg(level).arg(QString::fromLatin1(source))
                           .arg(pendingResistanceLevel));
        }
        return;
    }

    const bool confirmed = pendingResistanceLevel == level;
    if (changed || confirmed) {
        emit debug(QStringLiteral("Coach S resistance device-reported level=%1 via %2%3")
                       .arg(level).arg(QString::fromLatin1(source))
                       .arg(confirmed ? QStringLiteral(" (requested level confirmed)")
                                      : QString()));
        emit resistanceRead(static_cast<resistance_t>(level));
    }
    pendingResistanceLevel = -1;
    Resistance = level;
}

void kettlercoachrower::update() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState || !initDone) return;

    // Coasting raw watt samples do not keep power, speed or cadence alive.
    if (powerClock.isValid())
        updateStrokeIdle(powerClock.elapsed());

    // Writes are explicitly requested by QZ controls. No speculative
    // S-FIT 0x000B/0x001B-read/0x001C/0x001D polling: Coach S never answered.
    if (telemetrySeen && requestResistance != -1) {
        const resistance_t requested = requestResistance;
        requestResistance = -1;
        enqueueResistanceRequest(requested);
    }
    if (!requestInFlight && requestQueue.isEmpty()) {
        enqueueRead(DEVICE_DETAILS);
        enqueueRead(ROWING_TELEMETRY);
        enqueueRead(RESISTANCE_LEVEL);
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
               QString::number(activeRequest.function, 16) +
               QStringLiteral(" operation=0x") + QString::number(activeRequest.operation, 16));
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
