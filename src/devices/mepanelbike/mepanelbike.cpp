#include "mepanelbike.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
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

mepanelbike::mepanelbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &mepanelbike::update);
    refresh->start(200ms);
}

void mepanelbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

void mepanelbike::forceResistance(resistance_t requestResistance) {}

void mepanelbike::update() {
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
        update_metrics(false, watts());

        // sending poll every 2 seconds
        if (sec1Update++ >= (2000 / refresh->interval())) {
            sec1Update = 0;

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

void mepanelbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void mepanelbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    if (newValue.length() < 3)
        return;

    QString str;

    switch ((uint8_t)newValue.at(0)) {
    case 30: {
        double intValue =
            ((double)(((uint16_t)((uint8_t)newValue.at(1)) << 8) | (uint16_t)((uint8_t)newValue.at(2)))) / 100.0;
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = intValue;
        } else {
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        }

        break;
    }
    case 45:
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(1)) << 8) | (uint16_t)((uint8_t)newValue.at(2))));
        }
        break;
    case 60:
        // str = "RECEIVED_TIME,Min:" + newValue.at(1) + "  Sec:" + newValue.at(2);
        break;
    case 75:
        /*
        double intValue2 = (double)((newValue.at(1).intValue() * 256) + newValue.at(2).intValue());
        Double.isNaN(intValue2);
        str = "RECEIVED_DISTANCE," + (intValue2 / 10.0d);*/
        break;
    case 90:
        /*
        double intValue3 = (double)((newValue.at(1).intValue() * 256) + newValue.at(2).intValue());
        Double.isNaN(intValue3);
        str = "RECEIVED_CALORIES," + (intValue3 / 10.0d);*/
        break;
    case 105:
        m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(1)) << 8) | (uint16_t)((uint8_t)newValue.at(2)))) / 10.0;
        break;
    case 120:
        if (!disable_hr_frommachinery)
            Heart = newValue.at(1);
        break;
    case 135:
        // str = "RECEIVED_FAT_CHECK";
        break;
    case /*RECEIVED_HEART_REPLY*/ 150:
        // str = "RECEIVED_HEART_REPLY";
        break;
    case 165:
        // str = "RECEIVED_ERROR";
        break;
    case /*RECEIVED_REQUEST_PROGRAM*/ 180:

        break;
    case /*RECEIVED_REQUEST_STATE*/ 195:

        break;
    case /*RECEIVED_REQUEST_UNIT*/ 210:
        /*
        boolean z2 = newValue.at(1).intValue() == 0;
        if (newValue.at(2).intValue() != 1) {
            z = false;
        }
        str = "RECEIVED_REQUEST_UNIT,isInch:" + z2 + " isMale:" + z;*/
        break;
    case /*RECEIVED_REQUEST_HEIGHT*/ 225:
        // str = "RECEIVED_REQUEST_HEIGHT," + ((newValue.at(2).intValue() * 256) + newValue.at(1).intValue());
        break;
    case /*RECEIVED_REQUEST_WEIGHT*/ 226:
        // str = "RECEIVED_REQUEST_WEIGHT," + ((newValue.at(2).intValue() * 256) + newValue.at(1).intValue());
        break;
    case /*RECEIVED_REQUEST_AGE*/ 227:
        // str = "RECEIVED_REQUEST_AGE," + ((newValue.at(2).intValue() * 256) + newValue.at(1).intValue());
        break;
    default:
        str = "";
        break;
    }

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

    lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool() && disable_hr_frommachinery) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
    } else
#endif
    {
        if (heartRateBeltName.startsWith(QLatin1String("Disabled")) && disable_hr_frommachinery) {
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
    Resistance = Resistance.value();
    m_pelotonResistance = m_pelotonResistance.value();

    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

uint8_t mepanelbike::getCheckNum(uint8_t i, uint8_t i2) { return (i + i2); }

void mepanelbike::btinit() {
    uint8_t initData1[] = {0x1E, 0x11, 0x00, 0x00};
    initData1[3] = getCheckNum(initData1[1], initData1[2]);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void mepanelbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &mepanelbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &mepanelbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &mepanelbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &mepanelbike::descriptorWritten);

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
                qDebug() << QStringLiteral("creating virtual bike interface...");
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&mepanelbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &mepanelbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void mepanelbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void mepanelbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void mepanelbike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &mepanelbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void mepanelbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("mepanelbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void mepanelbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("mepanelbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void mepanelbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &mepanelbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &mepanelbike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &mepanelbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &mepanelbike::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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
}

bool mepanelbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t mepanelbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

void mepanelbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
