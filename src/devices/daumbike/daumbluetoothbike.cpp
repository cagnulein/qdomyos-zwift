#include "daumbluetoothbike.h"

#include "homeform.h"
#include "keepawakehelper.h"

#include <QDebug>
#include <initializer_list>
#include <chrono>

using namespace std::chrono_literals;

namespace {
constexpr uint8_t kInitialGear = 1;
constexpr int kProtocolGapMs = 60;

QByteArray makePacket(std::initializer_list<char> bytes) {
    QByteArray packet;
    for (const char byte : bytes)
        packet.append(byte);
    return packet;
}
}

daumbluetoothbike::daumbluetoothbike(bool noWriteResistance, bool noHeartService,
                                     int8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &daumbluetoothbike::update);
    refresh->start(200ms);

    initTimer = new QTimer(this);
    initTimer->setSingleShot(true);
    connect(initTimer, &QTimer::timeout, this, &daumbluetoothbike::sendInitPhase);
}

daumbluetoothbike::~daumbluetoothbike() {
    if (socket)
        socket->close();
}

bool daumbluetoothbike::matchesBluetoothName(const QString &name) {
    return name.compare(QStringLiteral("FS-IB50"), Qt::CaseInsensitive) == 0;
}

void daumbluetoothbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found Daum Bluetooth device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    if (!matchesBluetoothName(device.name()) || discoveryAgent)
        return;

    bluetoothDevice = device;
    discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this,
            &daumbluetoothbike::serviceDiscovered);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this,
            &daumbluetoothbike::serviceCanceled);
    connect(discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this,
            &daumbluetoothbike::serviceFinished);

    // Keep the Android workaround used by the existing Toorx/iConsole
    // Bluetooth 2.0 devices: leave "Toorx Treadmill Discovery Completed"
    // disabled to force FullDiscovery when the SPP service is not advertised.
    QSettings settings;
    const bool discoveryCompleted =
        settings.value(QZSettings::toorxtreadmill_discovery_completed,
                       QZSettings::default_toorxtreadmill_discovery_completed)
            .toBool();
    discoveryAgent->start(discoveryCompleted ? QBluetoothServiceDiscoveryAgent::MinimalDiscovery
                                             : QBluetoothServiceDiscoveryAgent::FullDiscovery);
}

void daumbluetoothbike::serviceCanceled() {
    emit debug(QStringLiteral("Daum Bluetooth service discovery canceled"));
}

void daumbluetoothbike::serviceDiscovered(const QBluetoothServiceInfo &service) {
    emit debug(QStringLiteral("Daum service: ") + service.serviceName() + QStringLiteral(" (") +
               service.serviceUuid().toString() + QStringLiteral(")"));
    if (found)
        return;

    const QBluetoothUuid serialUuid(QString::fromLatin1(serialPortUuid));
    const bool serialName = service.serviceName().startsWith(QStringLiteral("SerialPort")) ||
                            service.serviceName().startsWith(QStringLiteral("Serial Port"));
    if (service.serviceUuid() == serialUuid || serialName) {
        serialPortService = service;
        found = true;
        emit debug(QStringLiteral("Daum RFCOMM serial port service found"));
    }
}

void daumbluetoothbike::serviceFinished() {
    if (socket)
        return;

    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(socket, &QBluetoothSocket::readyRead, this, &daumbluetoothbike::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, &daumbluetoothbike::rfCommConnected);
    connect(socket, &QBluetoothSocket::disconnected, this, &daumbluetoothbike::disconnected);
    connect(socket, QOverload<QBluetoothSocket::SocketError>::of(&QBluetoothSocket::error), this,
            &daumbluetoothbike::onSocketErrorOccurred);

#ifdef Q_OS_ANDROID
    socket->setPreferredSecurityFlags(QBluetooth::NoSecurity);
#endif

    emit debug(QStringLiteral("Creating Daum RFCOMM socket"));
    if (found)
        socket->connectToService(serialPortService);
    else
        // Android 13 can omit the SPP service from discovery; this is the
        // same address/SerialPort workaround used by the other QZ classes.
        socket->connectToService(bluetoothDevice.address(), QBluetoothUuid(QBluetoothUuid::SerialPort));
}

void daumbluetoothbike::sendBytes(const QByteArray &bytes) {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState)
        return;
    qDebug() << "Daum BT TX:" << bytes.toHex(' ');
    emit debug(QStringLiteral("Daum BT >> ") + bytes.toHex(' '));
    socket->write(bytes);
}

void daumbluetoothbike::rfCommConnected() {
    emit debug(QStringLiteral("Connected to Daum Bluetooth SPP device ") + socket->peerName());
    socketReadBuffer.clear();
    initDone = false;
    initPhase = 0;
    cockpitAddress = 0;
    initTimer->start(400);
}

void daumbluetoothbike::sendInitPhase() {
    if (!socket || socket->state() != QBluetoothSocket::ConnectedState)
        return;

    QByteArray request;
    switch (initPhase) {
    case 0:
        request.append(static_cast<char>(0x11));
        break;
    case 1:
        request = makePacket({static_cast<char>(0x12), static_cast<char>(cockpitAddress)});
        break;
    case 2:
        request = makePacket({static_cast<char>(0x22), static_cast<char>(cockpitAddress)});
        break;
    case 3:
        request = makePacket({static_cast<char>(0x21), static_cast<char>(cockpitAddress)});
        break;
    case 4:
        request = makePacket({static_cast<char>(0x23), static_cast<char>(cockpitAddress), 0});
        break;
    case 5:
        request = makePacket({static_cast<char>(0x53), static_cast<char>(cockpitAddress),
                              static_cast<char>(kInitialGear)});
        break;
    default:
        return;
    }
    sendBytes(request);
}

void daumbluetoothbike::readSocket() {
    if (!socket)
        return;
    const QByteArray data = socket->readAll();
    if (data.isEmpty())
        return;
    socketReadBuffer.append(data);
    emit debug(QStringLiteral("Daum BT << ") + data.toHex(' '));
    processInput();
}

void daumbluetoothbike::processInput() {
    if (!initDone)
        processInitializationResponse();
    if (initDone)
        processTelemetry();
}

void daumbluetoothbike::processInitializationResponse() {
    if (initPhase == 0) {
        for (int i = 0; i + 1 < socketReadBuffer.size(); ++i) {
            if (static_cast<uint8_t>(socketReadBuffer.at(i)) == 0x11) {
                cockpitAddress = static_cast<uint8_t>(socketReadBuffer.at(i + 1));
                socketReadBuffer.remove(0, i + 2);
                initPhase = 1;
                initTimer->start(kProtocolGapMs);
                return;
            }
        }
        return;
    }

    const uint8_t command[] = {0x00, 0x12, 0x22, 0x21, 0x23, 0x53};
    const int length[] = {0, 2, 3, 3, 4, 3};
    if (initPhase < 1 || initPhase > 5)
        return;

    const QByteArray prefix = makePacket({static_cast<char>(command[initPhase]),
                                          static_cast<char>(cockpitAddress)});
    if (socketReadBuffer.size() < length[initPhase] ||
        !socketReadBuffer.startsWith(prefix))
        return;

    socketReadBuffer.remove(0, length[initPhase]);
    if (initPhase == 5) {
        initPhase = 6;
        initDone = true;
        emit debug(QStringLiteral("Daum Bluetooth initialization complete"));
        emit connectedAndDiscovered();
        return;
    }

    ++initPhase;
    initTimer->start(initPhase == 2 ? 1000 : kProtocolGapMs);
}

void daumbluetoothbike::processTelemetry() {
    for (int i = 0; i + telemetryLength <= socketReadBuffer.size(); ++i) {
        const uint8_t command = static_cast<uint8_t>(socketReadBuffer.at(i));
        const uint8_t address = static_cast<uint8_t>(socketReadBuffer.at(i + 1));
        if ((command == 0x40 || command == 0x41) && address == cockpitAddress) {
            const QByteArray packet = socketReadBuffer.mid(i, telemetryLength);
            socketReadBuffer.remove(0, i + telemetryLength);
            setTelemetry(packet);
            pollOutstanding = false;
            return;
        }
    }
}

void daumbluetoothbike::setTelemetry(const QByteArray &packet) {
    const uint8_t powerInFiveWatts = static_cast<uint8_t>(packet.at(5));
    const uint8_t cadence = static_cast<uint8_t>(packet.at(6));
    const uint8_t speed = static_cast<uint8_t>(packet.at(7));
    const uint16_t distanceInHundredMeters = static_cast<uint8_t>(packet.at(8)) |
        (static_cast<uint16_t>(static_cast<uint8_t>(packet.at(9))) << 8);
    const uint8_t heartRate = static_cast<uint8_t>(packet.at(14));
    const uint8_t gear = static_cast<uint8_t>(packet.at(16));

    devicePower = cadence == 0 ? 0.0 : static_cast<double>(powerInFiveWatts) * 5.0;
    deviceHeartRate = heartRate;
    deviceCadence = cadence;
    deviceSpeed = speed;
    deviceDistance = static_cast<double>(distanceInHundredMeters) * 100.0;
    deviceGear = gear;
}

void daumbluetoothbike::pollTelemetry() {
    sendBytes(makePacket({static_cast<char>(0x40), static_cast<char>(cockpitAddress)}));
    pollOutstanding = true;
}

int daumbluetoothbike::powerStep(double power) {
    int step = static_cast<int>((power + 2.5) / 5.0);
    return qBound(5, step, 160);
}

void daumbluetoothbike::writePowerTarget(double power) {
    const int step = powerStep(power);
    sendBytes(makePacket({static_cast<char>(0x51), static_cast<char>(cockpitAddress),
                          static_cast<char>(step)}));
    lastPowerStep = step;
}

resistance_t daumbluetoothbike::resistanceFromPowerRequest(uint16_t power) {
    if (power == 0)
        return 0;
    QSettings settings;
    double gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
    double offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
    double corrected = (power - offset) / gain;
    return qBound(static_cast<double>(min_resistance), corrected, static_cast<double>(max_resistance));
}

resistance_t daumbluetoothbike::pelotonToBikeResistance(int pelotonResistance) {
    return qBound(static_cast<double>(min_resistance), (pelotonResistance / 100.0) * max_resistance,
                  static_cast<double>(max_resistance));
}

void daumbluetoothbike::forceResistance(double requestResistance) {
    requestResistance = qBound(static_cast<double>(min_resistance), requestResistance,
                               static_cast<double>(max_resistance));
    const int step = powerStep(requestResistance);
    if (step == lastPowerStep || writePower)
        return;
    targetPower = requestResistance;
    writePower = true;
}

void daumbluetoothbike::innerWriteResistance() {
    if (requestResistance != -1) {
        requestResistance = static_cast<resistance_t>(qBound(
            static_cast<double>(min_resistance), static_cast<double>(requestResistance),
            static_cast<double>(max_resistance)));
        if (requestResistance != currentResistance().value())
            forceResistance(requestResistance);
        requestResistance = -1;
    }
    if (requestPower > 0) {
        forceResistance(requestPower);
        requestPower = -1;
    }
    if (requestInclination != -100) {
        forceInclination(requestInclination);
        requestInclination = -100;
    }
}

void daumbluetoothbike::update() {
    if (!initDone)
        return;

    QSettings settings;
    if (!this->hasVirtualDevice() &&
        settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
        emit debug(QStringLiteral("creating Daum Bluetooth virtual bike interface..."));
        virtualBike = new virtualbike(this, noWriteResistance, noHeartService,
                                      bikeResistanceOffset, bikeResistanceGain);
        connect(virtualBike, &virtualbike::changeInclination, this,
                &daumbluetoothbike::changeInclination);
        connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this,
                &daumbluetoothbike::ftmsCharacteristicChanged);
        this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
    }
    const QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    const bool ignoreHeart =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    if (writePower) {
        writePowerTarget(targetPower);
        writePower = false;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (!pollOutstanding && now - lastPollMs >= 1000) {
        pollTelemetry();
        lastPollMs = now;
    }

    const double power = devicePower;
    const double heartRate = deviceHeartRate;
    const double cadence = deviceCadence;
    const double speed = deviceSpeed;
    const double distance = deviceDistance;
    const uint8_t gear = deviceGear;

    if (gear >= 1 && gear <= 28 && m_gears != static_cast<double>(gear)) {
        m_gears = gear;
        if (settings.value(QZSettings::gears_restore_value, QZSettings::default_gears_restore_value).toBool())
            settings.setValue(QZSettings::gears_current_value, m_gears);
        if (homeform::singleton())
            homeform::singleton()->updateGearsValue();
        emit debug(QStringLiteral("Current Gear: ") + QString::number(m_gears));
    }

    Speed = speed;
    Distance = distance / 1000.0;
    Cadence = cadence;
    Resistance = power;
    m_watt = power;
    if (cadence > 0) {
        CrankRevs++;
        LastCrankEventTime += static_cast<uint16_t>(1024.0 / (cadence / 60.0));
    }
    if (watts()) {
        KCal += ((((0.048 * static_cast<double>(watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / static_cast<double>(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    }
    if (!ignoreHeart && heartRate > 0)
        Heart = heartRate;
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = static_cast<uint8_t>(KeepAwakeHelper::heart());
#endif
    if (ignoreHeart && heartRateBeltName.startsWith(QStringLiteral("Disabled")))
        update_hr_from_external();

    update_metrics(false, watts());
    innerWriteResistance();
    if (requestStart != -1) {
        requestStart = -1;
        emit bikeStarted();
    }
    if (requestStop != -1)
        requestStop = -1;
}

bool daumbluetoothbike::connected() {
    return socket && socket->state() == QBluetoothSocket::ConnectedState && initDone;
}

uint16_t daumbluetoothbike::watts() {
    return m_watt.value();
}

void daumbluetoothbike::changeInclination(double grade, double percentage) {
    bike::changeInclination(grade, percentage);
    Inclination = grade;
    m_currentSlopePercent = grade;
    m_slopeControlEnabled = true;
    updateSlopeTargetPower(true);
}

void daumbluetoothbike::forceInclination(double inclination) {
    Inclination = inclination;
    m_currentSlopePercent = inclination;
    m_slopeControlEnabled = true;
    updateSlopeTargetPower(true);
}

void daumbluetoothbike::onSocketErrorOccurred(QBluetoothSocket::SocketError error) {
    emit debug(QStringLiteral("Daum Bluetooth socket error: ") + QString::number(error));
}

void daumbluetoothbike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                   const QByteArray &newValue) {
    Q_UNUSED(characteristic)
    if (!newValue.isEmpty())
        m_lastFtmsCommand = static_cast<FtmsControlPointCommand>(newValue.at(0));
}
