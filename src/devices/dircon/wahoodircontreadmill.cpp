#include "wahoodircontreadmill.h"
#include "qzsettings.h"
#include <QSettings>
#include <QHostAddress>

wahoodircontreadmill::wahoodircontreadmill(const DirconDeviceInfo &deviceInfo, bool noWriteResistance,
                                           bool noHeartService)
    : treadmill(), deviceInfo(deviceInfo), tcpSocket(nullptr), isConnected(false), sequenceNumber(0),
      initDone(false), initRequest(false), servicesDiscovered(false), characteristicsDiscovered(false),
      noWriteResistance(noWriteResistance), noHeartService(noHeartService),
      lastSpeed(0.0), lastInclination(0.0) {

    qDebug() << "WahooDirconTreadmill: creating instance for" << deviceInfo.displayName
             << "at" << deviceInfo.address << ":" << deviceInfo.port;

    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, metric::METRIC_WATT_TREADMILL);
    Speed.setType(metric::METRIC_SPEED);

    // Initialize TCP socket
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::connected, this, &wahoodircontreadmill::onTcpConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &wahoodircontreadmill::onTcpDisconnected);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &wahoodircontreadmill::onTcpError);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &wahoodircontreadmill::onTcpDataReceived);

    // Initialize refresh timer
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &wahoodircontreadmill::update);
    refresh->start(settings.value(QZSettings::poll_device_time, QZSettings::default_poll_device_time).toInt());

    // Initialize connection timeout timer
    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);
    connect(connectionTimer, &QTimer::timeout, this, &wahoodircontreadmill::connectionTimeout);

    // Start connection
    connectToDevice();

#ifdef Q_OS_IOS
    h = new lockscreen();
    h->request();
#endif
}

wahoodircontreadmill::~wahoodircontreadmill() {
    qDebug() << "WahooDirconTreadmill: destroying instance";
    disconnectFromDevice();

#ifdef Q_OS_IOS
    if (h) {
        delete h;
    }
#endif
}

bool wahoodircontreadmill::connected() {
    return isConnected && initDone;
}

void wahoodircontreadmill::connectToDevice() {
    qDebug() << "WahooDirconTreadmill: connecting to" << deviceInfo.address << ":" << deviceInfo.port;

    if (tcpSocket->state() == QTcpSocket::ConnectedState ||
        tcpSocket->state() == QTcpSocket::ConnectingState) {
        qDebug() << "WahooDirconTreadmill: already connected or connecting";
        return;
    }

    tcpSocket->connectToHost(QHostAddress(deviceInfo.address), deviceInfo.port);
    connectionTimer->start(10000); // 10 seconds timeout
}

void wahoodircontreadmill::disconnectFromDevice() {
    qDebug() << "WahooDirconTreadmill: disconnecting";
    isConnected = false;
    initDone = false;

    if (tcpSocket) {
        tcpSocket->disconnectFromHost();
        if (tcpSocket->state() != QTcpSocket::UnconnectedState) {
            tcpSocket->waitForDisconnected(1000);
        }
    }

    if (connectionTimer) {
        connectionTimer->stop();
    }
}

void wahoodircontreadmill::onTcpConnected() {
    qDebug() << "WahooDirconTreadmill: TCP connected";
    connectionTimer->stop();
    isConnected = true;

    // Start DirCon protocol handshake
    sendDiscoverServices();
}

void wahoodircontreadmill::onTcpDisconnected() {
    qDebug() << "WahooDirconTreadmill: TCP disconnected";
    isConnected = false;
    initDone = false;
    servicesDiscovered = false;
    characteristicsDiscovered = false;

    emit disconnected();
}

void wahoodircontreadmill::onTcpError(QAbstractSocket::SocketError error) {
    qDebug() << "WahooDirconTreadmill: TCP error" << error << tcpSocket->errorString();
    isConnected = false;
    emit disconnected();
}

void wahoodircontreadmill::onTcpDataReceived() {
    QByteArray data = tcpSocket->readAll();
    receiveBuffer.append(data);

    // Parse DirCon packets from buffer
    while (receiveBuffer.size() >= DPKT_MESSAGE_HEADER_LENGTH) {
        DirconPacket packet;
        int result = packet.parse(receiveBuffer, sequenceNumber - 1);

        if (result == DPKT_PARSE_WAIT) {
            // Not enough data yet
            break;
        } else if (result == DPKT_PARSE_ERROR) {
            // Parse error, clear buffer and try again
            qDebug() << "WahooDirconTreadmill: packet parse error, clearing buffer";
            receiveBuffer.clear();
            break;
        } else {
            // Successfully parsed packet, remove it from buffer
            receiveBuffer.remove(0, result);
            processDirconPacket(packet);
        }
    }
}

void wahoodircontreadmill::connectionTimeout() {
    qDebug() << "WahooDirconTreadmill: connection timeout";
    disconnectFromDevice();
    emit disconnected();
}

void wahoodircontreadmill::sendDiscoverServices() {
    qDebug() << "WahooDirconTreadmill: discovering services";

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_DISCOVER_SERVICES;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconTreadmill: sent DISCOVER_SERVICES" << data.toHex(' ');
}

void wahoodircontreadmill::sendDiscoverCharacteristics() {
    qDebug() << "WahooDirconTreadmill: discovering characteristics for FTMS service";

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_DISCOVER_CHARACTERISTICS;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.uuid = UUID_FTMS_SERVICE;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconTreadmill: sent DISCOVER_CHARACTERISTICS" << data.toHex(' ');
}

void wahoodircontreadmill::sendSubscribeCharacteristic(quint16 uuid) {
    qDebug() << "WahooDirconTreadmill: subscribing to characteristic" << QString::number(uuid, 16);

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.uuid = uuid;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconTreadmill: sent SUBSCRIBE for UUID" << QString::number(uuid, 16) << data.toHex(' ');
}

void wahoodircontreadmill::sendReadCharacteristic(quint16 uuid) {
    qDebug() << "WahooDirconTreadmill: reading characteristic" << QString::number(uuid, 16);

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_READ_CHARACTERISTIC;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.uuid = uuid;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconTreadmill: sent READ for UUID" << QString::number(uuid, 16);
}

void wahoodircontreadmill::sendWriteCharacteristic(quint16 uuid, const QByteArray &data) {
    qDebug() << "WahooDirconTreadmill: writing characteristic" << QString::number(uuid, 16)
             << "data:" << data.toHex(' ');

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_WRITE_CHARACTERISTIC;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.uuid = uuid;
    packet.additional_data = data;
    packet.isRequest = true;

    QByteArray encodedData = packet.encode(sequenceNumber - 2);
    tcpSocket->write(encodedData);

    emit debug(QStringLiteral(" >> ") + data.toHex(' ') + QStringLiteral(" // FTMS Write"));
}

bool wahoodircontreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
                                               bool disable_log, bool wait_for_response) {
    Q_UNUSED(wait_for_response)

    if (!isConnected) {
        qDebug() << "WahooDirconTreadmill: not connected, cannot write";
        return false;
    }

    QByteArray payload((const char *)data, data_len);
    sendWriteCharacteristic(UUID_FITNESS_MACHINE_CONTROL_POINT, payload);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + payload.toHex(' ') + QStringLiteral(" // ") + info);
    }

    return true;
}

void wahoodircontreadmill::processDirconPacket(const DirconPacket &packet) {
    qDebug() << "WahooDirconTreadmill: received packet:" << QString(packet);

    switch (packet.Identifier) {
    case DPKT_MSGID_DISCOVER_SERVICES:
        handleDiscoverServicesResponse(packet);
        break;

    case DPKT_MSGID_DISCOVER_CHARACTERISTICS:
        handleDiscoverCharacteristicsResponse(packet);
        break;

    case DPKT_MSGID_READ_CHARACTERISTIC:
        handleReadCharacteristicResponse(packet);
        break;

    case DPKT_MSGID_UNSOLICITED_CHARACTERISTIC_NOTIFICATION:
        handleNotification(packet);
        break;

    case DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS:
        qDebug() << "WahooDirconTreadmill: notification enabled for UUID" << QString::number(packet.uuid, 16);
        break;

    case DPKT_MSGID_WRITE_CHARACTERISTIC:
        qDebug() << "WahooDirconTreadmill: write characteristic response";
        break;

    default:
        qDebug() << "WahooDirconTreadmill: unknown packet identifier" << packet.Identifier;
        break;
    }
}

void wahoodircontreadmill::handleDiscoverServicesResponse(const DirconPacket &packet) {
    qDebug() << "WahooDirconTreadmill: services discovered, count:" << packet.uuids.size();

    for (quint16 uuid : packet.uuids) {
        qDebug() << "WahooDirconTreadmill: found service UUID:" << QString::number(uuid, 16);
    }

    servicesDiscovered = true;

    // Discover characteristics for FTMS service
    sendDiscoverCharacteristics();
}

void wahoodircontreadmill::handleDiscoverCharacteristicsResponse(const DirconPacket &packet) {
    qDebug() << "WahooDirconTreadmill: characteristics discovered, count:" << packet.uuids.size();

    for (quint16 uuid : packet.uuids) {
        qDebug() << "WahooDirconTreadmill: found characteristic UUID:" << QString::number(uuid, 16);
    }

    characteristicsDiscovered = true;

    // Subscribe to notifications for important characteristics
    sendSubscribeCharacteristic(UUID_TREADMILL_DATA);
    sendSubscribeCharacteristic(UUID_FITNESS_MACHINE_STATUS);

    // Read fitness machine feature to understand capabilities
    sendReadCharacteristic(UUID_FITNESS_MACHINE_FEATURE);

    initDone = true;
    qDebug() << "WahooDirconTreadmill: initialization complete";
}

void wahoodircontreadmill::handleReadCharacteristicResponse(const DirconPacket &packet) {
    qDebug() << "WahooDirconTreadmill: read response for UUID" << QString::number(packet.uuid, 16)
             << "data:" << packet.additional_data.toHex(' ');

    if (packet.uuid == UUID_FITNESS_MACHINE_FEATURE) {
        parseFitnessMachineFeature(packet.additional_data);
    }
}

void wahoodircontreadmill::handleNotification(const DirconPacket &packet) {
    QDateTime now = QDateTime::currentDateTime();

    emit debug(QStringLiteral(" << ") + packet.additional_data.toHex(' ') +
               QStringLiteral(" // UUID: ") + QString::number(packet.uuid, 16));

    if (packet.uuid == UUID_TREADMILL_DATA) {
        parseTreadmillData(packet.additional_data);
        lastRefreshCharacteristicChanged = now;
    } else if (packet.uuid == UUID_HEART_RATE) {
        parseHeartRate(packet.additional_data);
    } else if (packet.uuid == UUID_FITNESS_MACHINE_STATUS) {
        parseFitnessMachineStatus(packet.additional_data);
    }
}

void wahoodircontreadmill::parseTreadmillData(const QByteArray &value) {
    QSettings settings;
    QDateTime now = QDateTime::currentDateTime();

    if (value.length() < 2) {
        qDebug() << "WahooDirconTreadmill: invalid Treadmill Data length";
        return;
    }

    union flags {
        struct {
            uint16_t moreData : 1;
            uint16_t avgSpeed : 1;
            uint16_t totDistance : 1;
            uint16_t inclination : 1;
            uint16_t rampAngle : 1;
            uint16_t posElevation : 1;
            uint16_t negElevation : 1;
            uint16_t instantPace : 1;
            uint16_t avgPace : 1;
            uint16_t expEnergy : 1;
            uint16_t heartRate : 1;
            uint16_t metabolic : 1;
            uint16_t elapsedTime : 1;
            uint16_t remainingTime : 1;
            uint16_t forceOnBelt : 1;
            uint16_t powerOutput : 1;
        };
        uint16_t word_flags;
    };

    flags Flags;
    int index = 0;
    Flags.word_flags = (value.at(1) << 8) | value.at(0);
    index += 2;

    // Instant Speed
    if (!Flags.moreData && index + 1 < value.length()) {
        Speed = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                          (uint16_t)((uint8_t)value.at(index)))) / 100.0;
        index += 2;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    }

    // Average Speed
    if (Flags.avgSpeed && index + 1 < value.length()) {
        double avgSpeed = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                                     (uint16_t)((uint8_t)value.at(index)))) / 100.0;
        index += 2;
        if (Flags.moreData) {
            Speed = avgSpeed;
        }
    }

    // Total Distance (skip)
    if (Flags.totDistance && index + 2 < value.length()) {
        index += 3;
    }

    // Calculate distance based on speed
    if (lastRefreshCharacteristicChanged.isValid()) {
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
    }

    // Inclination
    if (Flags.inclination && index + 1 < value.length()) {
        double incl = ((double)((int16_t)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                                          (uint16_t)((uint8_t)value.at(index))))) / 10.0;
        Inclination = incl;
        index += 2;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
    }

    // Ramp Angle (skip)
    if (Flags.rampAngle && index + 1 < value.length()) {
        index += 2;
    }

    // Positive Elevation Gain (skip)
    if (Flags.posElevation && index + 1 < value.length()) {
        index += 2;
    }

    // Negative Elevation Gain (skip)
    if (Flags.negElevation && index + 1 < value.length()) {
        index += 2;
    }

    // Instant Pace (skip)
    if (Flags.instantPace && index < value.length()) {
        index += 1;
    }

    // Average Pace (skip)
    if (Flags.avgPace && index < value.length()) {
        index += 1;
    }

    // Expanded Energy (Total, Per Hour, Per Minute)
    if (Flags.expEnergy && index + 5 < value.length()) {
        KCal = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                         (uint16_t)((uint8_t)value.at(index))));
        index += 6;
        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    }

    // Heart Rate
    if (Flags.heartRate && index < value.length()) {
        bool disable_hr_frommachinery =
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
        if (!disable_hr_frommachinery) {
            Heart = ((double)(((uint8_t)value.at(index))));
            emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
        }
        index += 1;
    }

    // Metabolic Equivalent
    if (Flags.metabolic && index < value.length()) {
        index += 1;
    }

    // Elapsed Time
    if (Flags.elapsedTime && index + 1 < value.length()) {
        index += 2;
    }

    // Remaining Time
    if (Flags.remainingTime && index + 1 < value.length()) {
        index += 2;
    }

    // Force on Belt (skip)
    if (Flags.forceOnBelt && index + 1 < value.length()) {
        index += 2;
    }

    // Power Output
    if (Flags.powerOutput && index + 1 < value.length()) {
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            m_watt = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                               (uint16_t)((uint8_t)value.at(index))));
        }
        index += 2;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
    }
}

void wahoodircontreadmill::parseHeartRate(const QByteArray &value) {
    QSettings settings;
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    if (disable_hr_frommachinery || value.length() < 2) {
        return;
    }

    uint8_t flags = value.at(0);
    bool hr_16bit = flags & 0x01;

    if (hr_16bit && value.length() >= 3) {
        Heart = ((double)(((uint16_t)((uint8_t)value.at(2)) << 8) |
                          (uint16_t)((uint8_t)value.at(1))));
    } else if (value.length() >= 2) {
        Heart = ((double)((uint8_t)value.at(1)));
    }

    emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
}

void wahoodircontreadmill::parseFitnessMachineFeature(const QByteArray &value) {
    qDebug() << "WahooDirconTreadmill: Fitness Machine Features:" << value.toHex(' ');

    if (value.length() < 8) {
        return;
    }

    // Parse feature flags to determine capabilities
    uint32_t features = ((uint32_t)((uint8_t)value.at(3)) << 24) |
                       ((uint32_t)((uint8_t)value.at(2)) << 16) |
                       ((uint32_t)((uint8_t)value.at(1)) << 8) |
                       ((uint32_t)((uint8_t)value.at(0)));

    qDebug() << "WahooDirconTreadmill: Feature flags:" << QString::number(features, 16);

    uint32_t targetSettings = ((uint32_t)((uint8_t)value.at(7)) << 24) |
                             ((uint32_t)((uint8_t)value.at(6)) << 16) |
                             ((uint32_t)((uint8_t)value.at(5)) << 8) |
                             ((uint32_t)((uint8_t)value.at(4)));

    qDebug() << "WahooDirconTreadmill: Target settings:" << QString::number(targetSettings, 16);
}

void wahoodircontreadmill::parseFitnessMachineStatus(const QByteArray &value) {
    if (value.length() < 1) {
        return;
    }

    uint8_t statusCode = value.at(0);
    qDebug() << "WahooDirconTreadmill: Fitness Machine Status:" << QString::number(statusCode, 16);
}

void wahoodircontreadmill::changeSpeed(double speed) {
    qDebug() << "WahooDirconTreadmill: change speed to" << speed;

    if (!isConnected || !initDone) {
        qDebug() << "WahooDirconTreadmill: not connected, cannot change speed";
        return;
    }

    forceSpeed(speed);
}

void wahoodircontreadmill::changeInclination(double grade, double percentage) {
    Q_UNUSED(grade)
    qDebug() << "WahooDirconTreadmill: change inclination to" << percentage;

    if (!isConnected || !initDone) {
        qDebug() << "WahooDirconTreadmill: not connected, cannot change inclination";
        return;
    }

    forceInclination(percentage);
}

void wahoodircontreadmill::forceSpeed(double speed) {
    qDebug() << "WahooDirconTreadmill: force speed to" << speed;

    if (lastSpeed == speed) {
        return;
    }

    lastSpeed = speed;

    // Send FTMS Set Target Speed command
    uint8_t ftms_data[3];
    ftms_data[0] = 0x02; // FTMS_SET_TARGET_SPEED
    uint16_t speedValue = (uint16_t)(speed * 100.0);
    ftms_data[1] = (uint8_t)(speedValue); // LSB
    ftms_data[2] = (uint8_t)(speedValue >> 8); // MSB

    writeCharacteristic(ftms_data, sizeof(ftms_data),
                       QStringLiteral("forceSpeed ") + QString::number(speed),
                       false, false);
}

void wahoodircontreadmill::forceInclination(double inclination) {
    qDebug() << "WahooDirconTreadmill: force inclination to" << inclination;

    if (lastInclination == inclination) {
        return;
    }

    lastInclination = inclination;

    // Send FTMS Set Target Inclination command
    uint8_t ftms_data[3];
    ftms_data[0] = 0x03; // FTMS_SET_TARGET_INCLINATION
    int16_t inclinationValue = (int16_t)(inclination * 10.0);
    ftms_data[1] = (uint8_t)(inclinationValue); // LSB
    ftms_data[2] = (uint8_t)(inclinationValue >> 8); // MSB

    writeCharacteristic(ftms_data, sizeof(ftms_data),
                       QStringLiteral("forceInclination ") + QString::number(inclination),
                       false, false);
}

uint16_t wahoodircontreadmill::watts(double weight) {
    Q_UNUSED(weight)
    // DirCon provides watts directly from the device
    return m_watt.value();
}

void wahoodircontreadmill::update() {
    if (!isConnected || !initDone) {
        return;
    }

    // Update metrics as needed
    // The actual data comes from notifications
}

void wahoodircontreadmill::init() {
    // Initialization is handled in the constructor and connection handshake
}

void wahoodircontreadmill::startDiscover() {
    // Not used for DirCon - discovery is handled by DirconDiscovery class
}
