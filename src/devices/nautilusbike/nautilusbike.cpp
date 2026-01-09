#include "nautilusbike.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>

#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

nautilusbike::nautilusbike(bool noWriteResistance, bool noHeartService, bool testResistance,
                           int8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);

    this->testResistance = testResistance;
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    initDone = false;
    connect(refresh, &QTimer::timeout, this, &nautilusbike::update);
    refresh->start(300ms);
}

nautilusbike::~nautilusbike() { qDebug() << QStringLiteral("~nautilusbike()"); }

void nautilusbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                       bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void nautilusbike::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void nautilusbike::update() {


if (!m_control)

    return;


    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        btinit(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && gattNotify2Characteristic.isValid() && initDone) {

        update_metrics(true, watts());

        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstVirtual && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this, &nautilusbike::changeInclinationRequested);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                firstVirtual = 1;
            }
        }
        // ********************************************************************************************************

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {

            sec1Update = 0;
        }

        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

            btinit(true);

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void nautilusbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void nautilusbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();

    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() == 20) {

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            uint8_t heart = ((uint8_t)newValue.at(16));
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) && heart != 0) {
                Heart = heart;
            }
        }

        Resistance = newValue.at(18);
        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        return;
    }

    if (newValue.length() != 14) {
        return;
    }

    double speed = GetSpeedFromPacket(newValue);
    m_watt = GetWattFromPacket(newValue);
    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    // double kcal = GetKcalFromPacket(newValue);
    // double distance = GetDistanceFromPacket(newValue) *
    // settings.value(QZSettings::domyos_elliptical_speed_ratio,
    // QZSettings::default_domyos_elliptical_speed_ratio).toDouble(); uint16_t watt = (newValue.at(13) << 8) |
    // newValue.at(14);

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = newValue.at(1);
    }

    Speed = speed;

    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

    CrankRevs++;
    LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    lastRefreshCharacteristicChanged = now;

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current inclination: ") + QString::number(Inclination.value()));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

double nautilusbike::GetSpeedFromPacket(const QByteArray &packet) {

    const double miles = 1.60934;
    uint16_t convertedData = 0;
    double data = 0;
    convertedData = (packet.at(4) << 8) | ((uint8_t)packet.at(3));
    data = (double)convertedData / 100.0f;
    if (!B616)
        data = data * miles;

    return data;
}

double nautilusbike::GetWattFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(7) << 8) | ((uint8_t)packet.at(6));
    return (double)convertedData / 100.0;
}

double nautilusbike::GetDistanceFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(12) << 8) | packet.at(13);
    double data = ((double)convertedData) / 10.0f;
    return data;
}

void nautilusbike::btinit(bool startTape) {

    QSettings settings;
    Q_UNUSED(startTape)

    uint8_t initData1[] = {0x07, 0x01, 0xd3, 0x00, 0x1f, 0x05, 0x01};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"));

    initDone = true;
}

void nautilusbike::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("1717b3c0-9803-11e3-90e1-0002a5d5c51b"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("a46a4a80-9803-11e3-8f3c-0002a5d5c51b"));
    QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("6be8f580-9803-11e3-ab03-0002a5d5c51b"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {

        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            qDebug() << QStringLiteral("char uuid") << c.uuid();
            auto descriptors_list = c.descriptors();
            for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
            }
        }

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);

        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        Q_ASSERT(gattNotify2Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &nautilusbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &nautilusbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &nautilusbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &nautilusbike::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);

        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void nautilusbike::searchingStop() { searchStopped = true; }

void nautilusbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void nautilusbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void nautilusbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("b1f93401-d4bd-4839-b7d7-b7434e9656cc"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (!gattCommunicationChannelService) {
        _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("f755c9cf-e1fc-4ecd-8d90-f2d7ebf56b81"));
        B616 = true;

        gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
        if (!gattCommunicationChannelService) {
            _gattCommunicationChannelServiceId = QBluetoothUuid(QStringLiteral("44f8d44f-7e03-4baf-9cc1-bd5a9c7a076b"));
            B616 = false;

            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            if (!gattCommunicationChannelService) {
                qDebug() << QStringLiteral("invalid service") << _gattCommunicationChannelServiceId.toString();
                return;
            }
        }
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &nautilusbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void nautilusbike::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("nautilusbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nautilusbike::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("nautilusbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nautilusbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');

    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &nautilusbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &nautilusbike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &nautilusbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &nautilusbike::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    searchStopped = false;
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Controller connected. Search services..."));
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("LowEnergy controller disconnected"));
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool nautilusbike::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void nautilusbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

uint16_t nautilusbike::watts() { return m_watt.value(); }


uint16_t nautilusbike::wattsFromResistance(double resistance) {
    // power table nautilus u626 #2118
    double intercept = 12.16860795336126;
    double coefCadence = 0.12260211;
    double coefResistance = -0.39240546;
    double coefCadenceSquared = 0.00464781;
    double coefCadenceResistance = 0.34516268;
    double coefResistanceSquared = -0.01031992;

    double wattage = intercept +
                     coefCadence * Cadence.average5s() +
                     coefResistance * resistance +
                     coefCadenceSquared * Cadence.average5s() * Cadence.average5s() +
                     coefCadenceResistance * Cadence.average5s() * resistance +
                     coefResistanceSquared * resistance * resistance;

    return wattage;
}

resistance_t nautilusbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.average5s();

    if (Cadence.average5s() == 0)
        return 1;

    for (resistance_t i = 1; i < maxResistance(); i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                     << wattsFromResistance(i + 1) << power;
            return i;
        }
    }
    if (power < wattsFromResistance(1))
        return 1;
    else
        return maxResistance();
}

resistance_t nautilusbike::maxResistance() {
    // power table nautilus u626 #2118
    return 25;
}
