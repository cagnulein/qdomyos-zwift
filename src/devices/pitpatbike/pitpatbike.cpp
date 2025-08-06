#include "pitpatbike.h"
#include "homeform.h"
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
#include <math.h>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

pitpatbike::pitpatbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                         double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &pitpatbike::update);
    refresh->start(200ms);
}

void pitpatbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                              bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                        QStringLiteral(" // ") + info;
    }

    loop.exec();
}

void pitpatbike::forceResistance(resistance_t requestResistance) {
    uint8_t noOpData[] = {0x6a, 0x06, 0x51, 0x82, 0x01, 0x01, 0xd5, 0x43};
    noOpData[5] = requestResistance;

    uint8_t crc = 0;
    for(int i = 0; i < sizeof(noOpData) - 2; i++) {
        crc ^= noOpData[i];
    }
    noOpData[6] = crc ^ 0x6A;

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("force resistance"), false, true);
}

void pitpatbike::sendPoll() {
    uint8_t noOpData[] = {0x6a, 0x05, 0xfd, 0xf8, 0x43};
    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
}

void pitpatbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        update_metrics(true, watts());

        // sending poll every 2 seconds
        if (sec1Update++ >= (1000 / refresh->interval())) {
            sec1Update = 0;
            sendPoll();
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance)
                requestResistance = max_resistance;
            else if (requestResistance <= 0)
                requestResistance = 1;

            if (requestResistance != currentResistance().value()) {
                qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void pitpatbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

resistance_t pitpatbike::pelotonToBikeResistance(int pelotonResistance) {
    for (resistance_t i = 1; i < max_resistance; i++) {
        if (bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i + 1) > pelotonResistance) {
            return i;
        }
    }
    if (pelotonResistance < bikeResistanceToPeloton(1))
        return 1;
    else
        return max_resistance;
}

double pitpatbike::bikeResistanceToPeloton(double resistance) {
    QSettings settings;
    // 0,0097x3 - 0,4972x2 + 10,126x - 37,08
    double p = ((pow(resistance, 3) * 0.0097) - (0.4972 * pow(resistance, 2)) + (10.126 * resistance) - 37.08);
    if (p < 0) {
        p = 0;
    }
    return (p * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
           settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
}

void pitpatbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    if (newValue.length() != 30) {
        return;
    }

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    double distance = GetDistanceFromPacket(newValue);

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = ((uint8_t)newValue.at(25));
    }
    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = 0.37497622 * ((double)Cadence.value());
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }

    m_watt = (uint16_t)((uint8_t)newValue.at(24)) + ((uint16_t)((uint8_t)newValue.at(23)) << 8);
    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
    } else
#endif
    {
        if (heartRateBeltName.startsWith(QLatin1String("Disabled"))) {
            update_hr_from_external();
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    // these useless lines are needed to calculate the AVG resistance and AVG peloton resistance since
    // echelon just send the resistance values when it changes
    Resistance = newValue.at(5);
    m_pelotonResistance = m_pelotonResistance.value();

    qDebug() << QStringLiteral("Current Local elapsed: ") + GetElapsedFromPacket(newValue).toString();
    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current Distance: ") + QString::number(distance);
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

QTime pitpatbike::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    QTime t(0, convertedData / 60, convertedData % 60);
    return t;
}

double pitpatbike::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

void pitpatbike::btinit() {
    initDone = true;
}

void pitpatbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((uint16_t)0xfbb1);
    QBluetoothUuid _gattNotify1CharacteristicId((uint16_t)0xfbb2);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &pitpatbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &pitpatbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &pitpatbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &pitpatbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                    if (virtual_device_rower) {
                        qDebug() << QStringLiteral("creating virtual rower interface...");
                        auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                        // connect(virtualRower,&virtualrower::debug ,this,&echelonrower::debug);
                        this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                    } else {
                        qDebug() << QStringLiteral("creating virtual bike interface...");
                        auto virtualBike =
                            new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                        // connect(virtualBike,&virtualbike::debug ,this,&pitpatbike::debug);
                        connect(virtualBike, &virtualbike::changeInclination, this, &pitpatbike::changeInclination);
                        this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                    }
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void pitpatbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void pitpatbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void pitpatbike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((uint16_t)0xfbb0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &pitpatbike::stateChanged);
    if(gattCommunicationChannelService != nullptr) {
        gattCommunicationChannelService->discoverDetails();
    } else {
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("Bluetooth Service Error! Restart the bike!");
        m_control->disconnectFromDevice();
    }
}

void pitpatbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("pitpatbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void pitpatbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("pitpatbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void pitpatbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &pitpatbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &pitpatbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &pitpatbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &pitpatbike::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
}

bool pitpatbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t pitpatbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

void pitpatbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
