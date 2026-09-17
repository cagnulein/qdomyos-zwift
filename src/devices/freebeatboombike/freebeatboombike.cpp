#include "freebeatboombike.h"

#include "keepawakehelper.h"
#include "virtualdevices/virtualbike.h"
#include <QMetaEnum>
#include <QSettings>
#include <QtMath>
#include <chrono>

using namespace std::chrono_literals;

namespace {
constexpr uint8_t syncByte = 0x55;
constexpr uint8_t dataTypes[] = {0x15, 0x03, 0x25};
constexpr uint8_t frameEnd = 0x3f;
constexpr uint8_t commandStart = 0x25;
constexpr uint8_t commandStream = 0x22;
constexpr uint8_t commandQuery = 0x08;
constexpr uint8_t commandResistance = 0x03;
constexpr uint8_t commandEnd = 0xa0;
constexpr char serviceUuid[] = "0000fff0-0000-1000-8000-00805f9b34fb";
constexpr char notifyUuid[] = "0000fff1-0000-1000-8000-00805f9b34fb";
constexpr char writeUuid[] = "0000fff2-0000-1000-8000-00805f9b34fb";
}

freebeatboombike::freebeatboombike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                   double bikeResistanceGain)
    : noWriteResistance(noWriteResistance), noHeartService(noHeartService),
      bikeResistanceOffset(bikeResistanceOffset), bikeResistanceGain(bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);

    refresh = new QTimer(this);
    connect(refresh, &QTimer::timeout, this, &freebeatboombike::update);
    refresh->start(200ms);
}

bool freebeatboombike::parseTelemetry(const QByteArray &packet, Telemetry *telemetry) {
    if (!telemetry || packet.size() < 13 || static_cast<uint8_t>(packet.at(0)) != syncByte ||
        static_cast<uint8_t>(packet.at(12)) != frameEnd) {
        return false;
    }

    const uint8_t type = static_cast<uint8_t>(packet.at(1));
    if (type != dataTypes[0] && type != dataTypes[1] && type != dataTypes[2]) {
        return false;
    }

    telemetry->resistance = static_cast<uint8_t>(packet.at(7));
    telemetry->rpm = static_cast<uint16_t>(static_cast<uint8_t>(packet.at(8))) |
                     (static_cast<uint16_t>(static_cast<uint8_t>(packet.at(9))) << 8);
    // The APK computes metres per second; QZ stores/display speed in km/h.
    // The Boom Bike's miles ratio is required by the device's calibration.
    telemetry->speed = telemetry->rpm * M_PI * wheelDiameter * 2.0 * wheelMultiplier * 3.6 * 0.62137 / 60.0;
    return true;
}

double freebeatboombike::distanceIncrement(double speedKmh, qint64 elapsedMsecs) {
    return speedKmh * static_cast<double>(elapsedMsecs) / 3600000.0;
}

QByteArray freebeatboombike::queryCommand() {
    QByteArray command;
    command.append(static_cast<char>(commandStart));
    command.append(static_cast<char>(commandQuery));
    command.append(static_cast<char>(0));
    command.append(static_cast<char>(commandQuery));
    command.append(static_cast<char>(commandEnd));
    return command;
}

QByteArray freebeatboombike::streamCommand(bool enabled) {
    const uint8_t value = enabled ? 1 : 0;
    QByteArray command;
    command.append(static_cast<char>(commandStart));
    command.append(static_cast<char>(commandStream));
    command.append(static_cast<char>(value));
    command.append(static_cast<char>((commandStream + value) & 0xff));
    command.append(static_cast<char>(commandEnd));
    return command;
}

QByteArray freebeatboombike::resistanceCommand(int resistance) {
    resistance = qBound(1, resistance, 100);
    QByteArray command;
    command.append(static_cast<char>(commandStart));
    command.append(static_cast<char>(commandResistance));
    command.append(static_cast<char>(resistance));
    command.append(static_cast<char>((commandResistance + resistance) & 0xff));
    command.append(static_cast<char>(commandEnd));
    return command;
}

void freebeatboombike::writeCharacteristic(const QByteArray &data, const QString &info) {
    if (!gattCommunicationChannelService || !gattWriteCharacteristic.isValid() ||
        gattCommunicationChannelService->state() != QLowEnergyService::ServiceDiscovered ||
        !m_control || m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("Freebeat Boom Bike write skipped: connection is not ready"));
        return;
    }

    const auto mode = (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse)
                          ? QLowEnergyService::WriteWithoutResponse
                          : QLowEnergyService::WriteWithResponse;
    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, data, mode);
    emit debug(QStringLiteral("Freebeat Boom Bike TX ") + data.toHex(' ') + QStringLiteral(" // ") + info);
}

void freebeatboombike::btinit() {
    initDone = true;
}

void freebeatboombike::forceResistance(resistance_t requestResistance) {
    if (noWriteResistance) {
        return;
    }

    writeCharacteristic(resistanceCommand(requestResistance), QStringLiteral("set resistance"));
}

void freebeatboombike::innerWriteResistance() {
    if (requestResistance == -1) {
        return;
    }

    requestResistance = qBound<resistance_t>(1, requestResistance, maxResistance);
    if (requestResistance != currentResistance().value()) {
        forceResistance(requestResistance);
    }
    requestResistance = -1;
}

void freebeatboombike::createVirtualBike() {
    if (hasVirtualDevice()) {
        return;
    }

    QSettings settings;
    if (!settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool()) {
        return;
    }

    emit debug(QStringLiteral("creating virtual bike interface..."));
    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
    connect(virtualBike, &virtualbike::changeInclination, this, &freebeatboombike::changeInclination);
    setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
}

void freebeatboombike::updateTelemetry(const Telemetry &telemetry) {
    const QDateTime now = QDateTime::currentDateTime();
    const qint64 elapsedMsecs = lastRefreshCharacteristicChanged.msecsTo(now);

    Speed = telemetry.speed;
    if (!firstCharacteristicChanged) {
        Distance += distanceIncrement(Speed.value(), elapsedMsecs);
    }

    Cadence = telemetry.rpm;
    Resistance = telemetry.resistance;
    m_pelotonResistance = telemetry.resistance;
    m_watt = telemetry.rpm * powerMultiplier * telemetry.resistance / 100.0;

    if (telemetry.rpm > 0) {
        CrankRevs++;
        LastCrankEventTime += static_cast<uint16_t>(1024.0 / (telemetry.rpm / 60.0));
    }

    firstCharacteristicChanged = false;
    lastRefreshCharacteristicChanged = now;
}

void freebeatboombike::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    if (gattNotifyCharacteristic.isValid() && characteristic.uuid() != gattNotifyCharacteristic.uuid()) {
        return;
    }

    Telemetry telemetry;
    if (!parseTelemetry(newValue, &telemetry)) {
        emit debug(QStringLiteral("Freebeat Boom Bike RX ignored ") + newValue.toHex(' '));
        return;
    }

    updateTelemetry(telemetry);
    emit debug(QStringLiteral("Freebeat Boom Bike RX ") + newValue.toHex(' ') +
               QStringLiteral(" rpm=") + QString::number(telemetry.rpm) +
               QStringLiteral(" resistance=") + QString::number(telemetry.resistance) +
               QStringLiteral(" speed=") + QString::number(telemetry.speed));
}

void freebeatboombike::update() {
    if (!m_control || m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
        return;
    }

    if (m_control->state() != QLowEnergyController::DiscoveredState || !initDone) {
        return;
    }

    createVirtualBike();

    // The official Boom Bike app polls with MACHINE_QUERY every ~150 ms;
    // enabling the repeat-stream command alone only returns a status frame.
    writeCharacteristic(queryCommand(), QStringLiteral("query telemetry"));

    QSettings settings;
    const QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    const bool disableHeart =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = static_cast<uint8_t>(KeepAwakeHelper::heart());
    } else
#endif
        if (disableHeart && heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        update_hr_from_external();
    }

    update_metrics(false, watts());
    innerWriteResistance();

    if (requestStart != -1) {
        requestStart = -1;
        emit bikeStarted();
    }
    if (requestStop != -1) {
        requestStop = -1;
    }
}

bool freebeatboombike::connected() { return initDone; }

uint16_t freebeatboombike::watts() { return static_cast<uint16_t>(m_watt.value()); }

resistance_t freebeatboombike::pelotonToBikeResistance(int pelotonResistance) {
    return qBound<resistance_t>(1, pelotonResistance, maxResistance);
}

resistance_t freebeatboombike::resistanceFromPowerRequest(uint16_t power) {
    const double resistance = power / (80.0 * powerMultiplier / 100.0);
    return qBound<resistance_t>(1, qRound(resistance), maxResistance);
}

void freebeatboombike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("Freebeat Boom Bike service discovered ") + gatt.toString());
}

void freebeatboombike::serviceScanDone() {
    const QBluetoothUuid serviceId(QString::fromLatin1(serviceUuid));
    gattCommunicationChannelService = m_control->createServiceObject(serviceId, this);
    if (!gattCommunicationChannelService) {
        emit debug(QStringLiteral("Freebeat Boom Bike service fff0 not found"));
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &freebeatboombike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void freebeatboombike::stateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered) {
        return;
    }

    gattWriteCharacteristic =
        gattCommunicationChannelService->characteristic(QBluetoothUuid(QString::fromLatin1(writeUuid)));
    gattNotifyCharacteristic =
        gattCommunicationChannelService->characteristic(QBluetoothUuid(QString::fromLatin1(notifyUuid)));
    if (!gattWriteCharacteristic.isValid() || !gattNotifyCharacteristic.isValid()) {
        emit debug(QStringLiteral("Freebeat Boom Bike fff1/fff2 characteristic missing"));
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
            &freebeatboombike::characteristicChanged);
    connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
            &freebeatboombike::characteristicWritten);
    connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
            &freebeatboombike::descriptorWritten);
    connect(gattCommunicationChannelService,
            static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
            this, &freebeatboombike::errorService);

    QByteArray descriptor;
    descriptor.append(static_cast<char>(0x01));
    descriptor.append(static_cast<char>(0x00));
    gattCommunicationChannelService->writeDescriptor(
        gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
}

void freebeatboombike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("Freebeat Boom Bike descriptor written ") + descriptor.name() + QStringLiteral(" ") +
               newValue.toHex(' '));
    initRequest = true;
    emit connectedAndDiscovered();
}

void freebeatboombike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("Freebeat Boom Bike characteristic written ") + newValue.toHex(' '));
}

void freebeatboombike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    if (state == QLowEnergyController::UnconnectedState) {
        initDone = false;
        firstCharacteristicChanged = true;
        Speed = 0;
    }
}

void freebeatboombike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found Freebeat Boom Bike: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    bluetoothDevice = device;
    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &freebeatboombike::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &freebeatboombike::serviceScanDone);
    connect(m_control, &QLowEnergyController::stateChanged, this, &freebeatboombike::controllerStateChanged);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &freebeatboombike::error);
    connect(m_control, &QLowEnergyController::connected, this, [this]() {
        emit debug(QStringLiteral("Freebeat Boom Bike controller connected"));
        m_control->discoverServices();
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        emit debug(QStringLiteral("Freebeat Boom Bike controller disconnected"));
        emit disconnected();
    });
    m_control->connectToDevice();
}

void freebeatboombike::error(QLowEnergyController::Error err) {
    emit debug(QStringLiteral("Freebeat Boom Bike controller error ") + QString::number(err));
}

void freebeatboombike::errorService(QLowEnergyService::ServiceError err) {
    emit debug(QStringLiteral("Freebeat Boom Bike service error ") + QString::number(err));
}
