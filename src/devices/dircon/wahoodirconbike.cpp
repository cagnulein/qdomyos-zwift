#include "wahoodirconbike.h"
#include "qzsettings.h"
#include <QSettings>
#include <QHostAddress>

wahoodirconbike::wahoodirconbike(const DirconDeviceInfo &deviceInfo, bool noWriteResistance, bool noHeartService,
                                 int8_t bikeResistanceOffset, double bikeResistanceGain)
    : bike(), deviceInfo(deviceInfo), tcpSocket(nullptr), isConnected(false), sequenceNumber(0),
      initDone(false), initRequest(false), servicesDiscovered(false), characteristicsDiscovered(false),
      noWriteResistance(noWriteResistance), noHeartService(noHeartService),
      bikeResistanceOffset(bikeResistanceOffset), bikeResistanceGain(bikeResistanceGain),
      max_resistance(32), lastResistance(0), lastRequestResistance(0) {

    qDebug() << "WahooDirconBike: creating instance for" << deviceInfo.displayName
             << "at" << deviceInfo.address << ":" << deviceInfo.port;

    QSettings settings;
    m_watt.setType(metric::METRIC_WATT, metric::METRIC_WATT_BIKE);
    Speed.setType(metric::METRIC_SPEED);

    // Initialize TCP socket
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::connected, this, &wahoodirconbike::onTcpConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &wahoodirconbike::onTcpDisconnected);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &wahoodirconbike::onTcpError);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &wahoodirconbike::onTcpDataReceived);

    // Initialize refresh timer
    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &wahoodirconbike::update);
    refresh->start(settings.value(QZSettings::poll_device_time, QZSettings::default_poll_device_time).toInt());

    // Initialize connection timeout timer
    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);
    connect(connectionTimer, &QTimer::timeout, this, &wahoodirconbike::connectionTimeout);

    // Start connection
    connectToDevice();

#ifdef Q_OS_IOS
    h = new lockscreen();
    h->request();
#endif
}

wahoodirconbike::~wahoodirconbike() {
    qDebug() << "WahooDirconBike: destroying instance";
    disconnectFromDevice();

#ifdef Q_OS_IOS
    if (h) {
        delete h;
    }
#endif
}

bool wahoodirconbike::connected() {
    return isConnected && initDone;
}

void wahoodirconbike::connectToDevice() {
    qDebug() << "WahooDirconBike: connecting to" << deviceInfo.address << ":" << deviceInfo.port;

    if (tcpSocket->state() == QTcpSocket::ConnectedState ||
        tcpSocket->state() == QTcpSocket::ConnectingState) {
        qDebug() << "WahooDirconBike: already connected or connecting";
        return;
    }

    tcpSocket->connectToHost(QHostAddress(deviceInfo.address), deviceInfo.port);
    connectionTimer->start(10000); // 10 seconds timeout
}

void wahoodirconbike::disconnectFromDevice() {
    qDebug() << "WahooDirconBike: disconnecting";
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

void wahoodirconbike::onTcpConnected() {
    qDebug() << "WahooDirconBike: TCP connected";
    connectionTimer->stop();
    isConnected = true;

    // Start DirCon protocol handshake
    sendDiscoverServices();
}

void wahoodirconbike::onTcpDisconnected() {
    qDebug() << "WahooDirconBike: TCP disconnected";
    isConnected = false;
    initDone = false;
    servicesDiscovered = false;
    characteristicsDiscovered = false;

    emit disconnected();
}

void wahoodirconbike::onTcpError(QAbstractSocket::SocketError error) {
    qDebug() << "WahooDirconBike: TCP error" << error << tcpSocket->errorString();
    isConnected = false;
    emit disconnected();
}

void wahoodirconbike::onTcpDataReceived() {
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
            qDebug() << "WahooDirconBike: packet parse error, clearing buffer";
            receiveBuffer.clear();
            break;
        } else {
            // Successfully parsed packet, remove it from buffer
            receiveBuffer.remove(0, result);
            processDirconPacket(packet);
        }
    }
}

void wahoodirconbike::connectionTimeout() {
    qDebug() << "WahooDirconBike: connection timeout";
    disconnectFromDevice();
    emit disconnected();
}

void wahoodirconbike::sendDiscoverServices() {
    qDebug() << "WahooDirconBike: discovering services";

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_DISCOVER_SERVICES;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconBike: sent DISCOVER_SERVICES" << data.toHex(' ');
}

void wahoodirconbike::sendDiscoverCharacteristics() {
    qDebug() << "WahooDirconBike: discovering characteristics for FTMS service";

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_DISCOVER_CHARACTERISTICS;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.uuid = UUID_FTMS_SERVICE;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconBike: sent DISCOVER_CHARACTERISTICS" << data.toHex(' ');
}

void wahoodirconbike::sendSubscribeCharacteristic(quint16 uuid) {
    qDebug() << "WahooDirconBike: subscribing to characteristic" << QString::number(uuid, 16);

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_ENABLE_CHARACTERISTIC_NOTIFICATIONS;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.uuid = uuid;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconBike: sent SUBSCRIBE for UUID" << QString::number(uuid, 16) << data.toHex(' ');
}

void wahoodirconbike::sendReadCharacteristic(quint16 uuid) {
    qDebug() << "WahooDirconBike: reading characteristic" << QString::number(uuid, 16);

    DirconPacket packet;
    packet.MessageVersion = 1;
    packet.Identifier = DPKT_MSGID_READ_CHARACTERISTIC;
    packet.SequenceNumber = sequenceNumber++;
    packet.ResponseCode = DPKT_RESPCODE_SUCCESS_REQUEST;
    packet.uuid = uuid;
    packet.isRequest = true;

    QByteArray data = packet.encode(sequenceNumber - 2);
    tcpSocket->write(data);

    qDebug() << "WahooDirconBike: sent READ for UUID" << QString::number(uuid, 16);
}

void wahoodirconbike::sendWriteCharacteristic(quint16 uuid, const QByteArray &data) {
    qDebug() << "WahooDirconBike: writing characteristic" << QString::number(uuid, 16)
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

bool wahoodirconbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
                                         bool disable_log, bool wait_for_response) {
    Q_UNUSED(wait_for_response)

    if (!isConnected) {
        qDebug() << "WahooDirconBike: not connected, cannot write";
        return false;
    }

    QByteArray payload((const char *)data, data_len);
    sendWriteCharacteristic(UUID_FITNESS_MACHINE_CONTROL_POINT, payload);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + payload.toHex(' ') + QStringLiteral(" // ") + info);
    }

    return true;
}

void wahoodirconbike::processDirconPacket(const DirconPacket &packet) {
    qDebug() << "WahooDirconBike: received packet:" << QString(packet);

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
        qDebug() << "WahooDirconBike: notification enabled for UUID" << QString::number(packet.uuid, 16);
        break;

    case DPKT_MSGID_WRITE_CHARACTERISTIC:
        qDebug() << "WahooDirconBike: write characteristic response";
        break;

    default:
        qDebug() << "WahooDirconBike: unknown packet identifier" << packet.Identifier;
        break;
    }
}

void wahoodirconbike::handleDiscoverServicesResponse(const DirconPacket &packet) {
    qDebug() << "WahooDirconBike: services discovered, count:" << packet.uuids.size();

    for (quint16 uuid : packet.uuids) {
        qDebug() << "WahooDirconBike: found service UUID:" << QString::number(uuid, 16);
    }

    servicesDiscovered = true;

    // Discover characteristics for FTMS service
    sendDiscoverCharacteristics();
}

void wahoodirconbike::handleDiscoverCharacteristicsResponse(const DirconPacket &packet) {
    qDebug() << "WahooDirconBike: characteristics discovered, count:" << packet.uuids.size();

    for (quint16 uuid : packet.uuids) {
        qDebug() << "WahooDirconBike: found characteristic UUID:" << QString::number(uuid, 16);
    }

    characteristicsDiscovered = true;

    // Subscribe to notifications for important characteristics
    sendSubscribeCharacteristic(UUID_INDOOR_BIKE_DATA);
    sendSubscribeCharacteristic(UUID_FITNESS_MACHINE_STATUS);

    // Read fitness machine feature to understand capabilities
    sendReadCharacteristic(UUID_FITNESS_MACHINE_FEATURE);

    initDone = true;
    qDebug() << "WahooDirconBike: initialization complete";
}

void wahoodirconbike::handleReadCharacteristicResponse(const DirconPacket &packet) {
    qDebug() << "WahooDirconBike: read response for UUID" << QString::number(packet.uuid, 16)
             << "data:" << packet.additional_data.toHex(' ');

    if (packet.uuid == UUID_FITNESS_MACHINE_FEATURE) {
        parseFitnessMachineFeature(packet.additional_data);
    }
}

void wahoodirconbike::handleNotification(const DirconPacket &packet) {
    QDateTime now = QDateTime::currentDateTime();

    emit debug(QStringLiteral(" << ") + packet.additional_data.toHex(' ') +
               QStringLiteral(" // UUID: ") + QString::number(packet.uuid, 16));

    if (packet.uuid == UUID_INDOOR_BIKE_DATA) {
        parseIndoorBikeData(packet.additional_data);
        lastRefreshCharacteristicChanged = now;
    } else if (packet.uuid == UUID_CYCLING_POWER_MEASUREMENT) {
        parseCyclingPowerMeasurement(packet.additional_data);
    } else if (packet.uuid == UUID_HEART_RATE) {
        parseHeartRate(packet.additional_data);
    } else if (packet.uuid == UUID_FITNESS_MACHINE_STATUS) {
        parseFitnessMachineStatus(packet.additional_data);
    }
}

void wahoodirconbike::parseIndoorBikeData(const QByteArray &value) {
    QSettings settings;
    QDateTime now = QDateTime::currentDateTime();

    if (value.length() < 2) {
        qDebug() << "WahooDirconBike: invalid Indoor Bike Data length";
        return;
    }

    union flags {
        struct {
            uint16_t moreData : 1;
            uint16_t avgSpeed : 1;
            uint16_t instantCadence : 1;
            uint16_t avgCadence : 1;
            uint16_t totDistance : 1;
            uint16_t resistanceLvl : 1;
            uint16_t instantPower : 1;
            uint16_t avgPower : 1;
            uint16_t expEnergy : 1;
            uint16_t heartRate : 1;
            uint16_t metabolic : 1;
            uint16_t elapsedTime : 1;
            uint16_t remainingTime : 1;
            uint16_t spare : 3;
        };
        uint16_t word_flags;
    };

    flags Flags;
    int index = 0;
    Flags.word_flags = (value.at(1) << 8) | value.at(0);
    index += 2;

    // Instant Speed
    if (!Flags.moreData && index + 1 < value.length()) {
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)value.at(index)))) / 100.0;
        } else {
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }
        index += 2;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    }

    // Average Speed
    if (Flags.avgSpeed && index + 1 < value.length()) {
        double avgSpeed = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                                     (uint16_t)((uint8_t)value.at(index)))) / 100.0;
        index += 2;
        if (Flags.moreData && !settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = avgSpeed;
        }
    }

    // Instant Cadence
    if (Flags.instantCadence && index + 1 < value.length()) {
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                                (uint16_t)((uint8_t)value.at(index)))) / 2.0;
        }
        index += 2;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    }

    // Average Cadence
    if (Flags.avgCadence && index + 1 < value.length()) {
        double avgCadence = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                                       (uint16_t)((uint8_t)value.at(index)))) / 2.0;
        index += 2;
        if (!Flags.instantCadence &&
            settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = avgCadence;
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

    // Resistance Level
    if (Flags.resistanceLvl && index + 1 < value.length()) {
        double d = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)value.at(index))));
        index += 2;
        if (d > 0) {
            Resistance = d;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
            emit resistanceRead(Resistance.value());
        }
    }

    // Instant Power
    if (Flags.instantPower && index + 1 < value.length()) {
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            m_watt = ((double)(((uint16_t)((uint8_t)value.at(index + 1)) << 8) |
                               (uint16_t)((uint8_t)value.at(index))));
        }
        index += 2;
        emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
    }

    // Average Power
    if (Flags.avgPower && index + 1 < value.length()) {
        index += 2;
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
}

void wahoodirconbike::parseCyclingPowerMeasurement(const QByteArray &value) {
    QSettings settings;

    if (value.length() < 4) {
        return;
    }

    // Parse cycling power measurement (0x2A63)
    uint16_t flags = (value.at(1) << 8) | value.at(0);
    bool power_present = !(flags & 0x01);

    if (power_present) {
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            m_watt = ((double)(((uint16_t)((uint8_t)value.at(3)) << 8) |
                               (uint16_t)((uint8_t)value.at(2))));
            emit debug(QStringLiteral("Current Watt (from cycling power): ") + QString::number(m_watt.value()));
        }
    }
}

void wahoodirconbike::parseHeartRate(const QByteArray &value) {
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

void wahoodirconbike::parseFitnessMachineFeature(const QByteArray &value) {
    qDebug() << "WahooDirconBike: Fitness Machine Features:" << value.toHex(' ');

    if (value.length() < 8) {
        return;
    }

    // Parse feature flags to determine capabilities
    // This helps us understand what the bike supports
    uint32_t features = ((uint32_t)((uint8_t)value.at(3)) << 24) |
                       ((uint32_t)((uint8_t)value.at(2)) << 16) |
                       ((uint32_t)((uint8_t)value.at(1)) << 8) |
                       ((uint32_t)((uint8_t)value.at(0)));

    qDebug() << "WahooDirconBike: Feature flags:" << QString::number(features, 16);

    // Target settings features
    uint32_t targetSettings = ((uint32_t)((uint8_t)value.at(7)) << 24) |
                             ((uint32_t)((uint8_t)value.at(6)) << 16) |
                             ((uint32_t)((uint8_t)value.at(5)) << 8) |
                             ((uint32_t)((uint8_t)value.at(4)));

    qDebug() << "WahooDirconBike: Target settings:" << QString::number(targetSettings, 16);
}

void wahoodirconbike::parseFitnessMachineStatus(const QByteArray &value) {
    if (value.length() < 1) {
        return;
    }

    uint8_t statusCode = value.at(0);
    qDebug() << "WahooDirconBike: Fitness Machine Status:" << QString::number(statusCode, 16);

    // Handle status codes as needed
    // 0x01 = Reset
    // 0x02 = Stopped or Paused by User
    // 0x03 = Stopped by Safety Key
    // 0x04 = Started or Resumed by User
    // etc.
}

void wahoodirconbike::forceResistance(resistance_t requestResistance) {
    qDebug() << "WahooDirconBike: force resistance to" << requestResistance;

    if (lastRequestResistance == requestResistance) {
        return;
    }

    lastRequestResistance = requestResistance;

    // Send FTMS Set Target Resistance Level command
    uint8_t ftms_data[3];
    ftms_data[0] = 0x04; // FTMS_SET_TARGET_RESISTANCE_LEVEL
    ftms_data[1] = (uint8_t)(requestResistance * 10.0); // LSB
    ftms_data[2] = (uint8_t)((int)(requestResistance * 10.0) >> 8); // MSB

    writeCharacteristic(ftms_data, sizeof(ftms_data),
                       QStringLiteral("forceResistance ") + QString::number(requestResistance),
                       false, false);
}

void wahoodirconbike::forcePower(int16_t requestPower) {
    qDebug() << "WahooDirconBike: force power to" << requestPower;

    // Send FTMS Set Target Power command
    uint8_t ftms_data[3];
    ftms_data[0] = 0x05; // FTMS_SET_TARGET_POWER
    ftms_data[1] = (uint8_t)(requestPower); // LSB
    ftms_data[2] = (uint8_t)(requestPower >> 8); // MSB

    writeCharacteristic(ftms_data, sizeof(ftms_data),
                       QStringLiteral("forcePower ") + QString::number(requestPower),
                       false, false);
}

uint16_t wahoodirconbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

uint16_t wahoodirconbike::wattsFromResistance(double resistance) {
    // Simple power estimation formula
    // This should be calibrated based on actual device behavior
    return (uint16_t)(resistance * 10.0);
}

void wahoodirconbike::update() {
    if (!isConnected || !initDone) {
        return;
    }

    // Update metrics as needed
    // The actual data comes from notifications, so this mostly handles
    // state management and timeout checks
}

void wahoodirconbike::init() {
    // Initialization is handled in the constructor and connection handshake
}

void wahoodirconbike::startDiscover() {
    // Not used for DirCon - discovery is handled by DirconDiscovery class
}

resistance_t wahoodirconbike::pelotonToBikeResistance(int pelotonResistance) {
    return (resistance_t)pelotonResistance / 100.0 * max_resistance;
}

resistance_t wahoodirconbike::resistanceFromPowerRequest(uint16_t power) {
    // Simple estimation - should be calibrated
    return (resistance_t)(power / 10.0);
}
