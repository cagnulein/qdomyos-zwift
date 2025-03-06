#include "solebike.h"
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

solebike::solebike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
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
    connect(refresh, &QTimer::timeout, this, &solebike::update);
    refresh->start(300ms);
}

void solebike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info;
    }

    loop.exec();
}

void solebike::forceResistance(resistance_t requestResistance) {
    uint8_t up[] = {0x5b, 0x02, 0xf1, 0x02, 0x5d};
    uint8_t down[] = {0x5b, 0x02, 0xf1, 0x03, 0x5d};

    /*if (requestSpeed > Speed.value()) {
        if (requestSpeedState == IDLE)
            requestSpeedState = UP;
        else if (requestSpeedState == DOWN) {
            requestSpeedState = IDLE;
            this->requestSpeed = -1;
            return;
        }

        if (gattCustomService)
            writeCharacteristic(up, sizeof(up), QStringLiteral("speed up"), false, true);
    } else if (requestSpeed < Speed.value()) {
        if (requestSpeedState == IDLE)
            requestSpeedState = DOWN;
        else if (requestSpeedState == UP) {
            requestSpeedState = IDLE;
            this->requestSpeed = -1;
            return;
        }
        if (gattCustomService)
            writeCharacteristic(down, sizeof(down), QStringLiteral("speed down"), false, true);
    } else {
        this->requestSpeed = -1;
        requestSpeedState = IDLE;
    }*/
    writeCharacteristic(up, sizeof(up), QStringLiteral("up"), false, true);
}

void solebike::update() {

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotifyCharacteristic.isValid() && initDone) {

        update_metrics(false, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {

            sec1Update = 0;
        } else {
            if (!r92) {
                uint8_t noOpData[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
                uint8_t noOpData1[] = {0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d};
                uint8_t noOpData2[] = {0x5b, 0x04, 0x00, 0x13, 0x4f, 0x4b, 0x5d};
                switch (counterPoll) {

                case 0:
                    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                    break;
                case 1:
                    writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                    break;
                case 2:
                    writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"), false, true);
                    break;
                }
            } else {
                uint8_t noOpData[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
                uint8_t noOpData1[] = {0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d};
                uint8_t noOpData2[] = {0x5b, 0x04, 0x00, 0x14, 0x4f, 0x4b, 0x5d};
                switch (counterPoll) {

                case 0:
                    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                    break;
                case 1:
                    writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                    break;
                case 2:
                    writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"), false, true);
                    break;
                }
            }
            counterPoll++;
            if (counterPoll > 2) {
                counterPoll = 0;
            }
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

void solebike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

resistance_t solebike::pelotonToBikeResistance(int pelotonResistance) {
    for (resistance_t i = 1; i < max_resistance; i++) {
        if (bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i + 1) >= pelotonResistance) {
            return i;
        }
    }
    if (pelotonResistance < bikeResistanceToPeloton(1))
        return 1;
    else
        return max_resistance;
}

double solebike::bikeResistanceToPeloton(double resistance) {
    // 0,0097x3 - 0,4972x2 + 10,126x - 37,08
    double p = ((pow(resistance, 3) * 0.0097) - (0.4972 * pow(resistance, 2)) + (10.126 * resistance) - 37.08);
    if (p < 0) {
        p = 0;
    }
    return p;
}

void solebike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    // resistance value is in another frame
    if (newValue.length() == 5 && ((unsigned char)newValue.at(0)) == 0x5b && ((unsigned char)newValue.at(1)) == 0x02 &&
        ((unsigned char)newValue.at(2)) == 0x13) {
        double res = 1.0;
        if (newValue.at(3) > 0)
            res = newValue.at(3);
        Resistance = res;
        emit resistanceRead(Resistance.value());
        m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());

        qDebug() << QStringLiteral("Current resistance: ") + QString::number(Resistance.value());
        return;
    }

    if (newValue.length() < 20) {
        return;
    }

    double distance = GetDistanceFromPacket(newValue);

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = ((uint8_t)newValue.at(10));
    }
    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = GetSpeedFromPacket(newValue);
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }

    m_watt = GetWattFromPacket(newValue);

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
    Resistance = Resistance.value();
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

QTime solebike::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    QTime t(0, convertedData / 60, convertedData % 60);
    return t;
}

double solebike::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

double solebike::GetWattFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(13) << 8) | packet.at(14);
    double data = ((double)convertedData);
    return data;
}

double solebike::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(11) << 8) | packet.at(12);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

void solebike::btinit() {

    if (!r92) {
        uint8_t initData1[] = {0x5b, 0x01, 0xf0, 0x5d};
        uint8_t initData2[] = {0x5b, 0x02, 0x03, 0x01, 0x5d};
        uint8_t initData3[] = {0x5b, 0x06, 0x07, 0x01, 0x23, 0x00, 0x9b, 0x43, 0x5d};
        uint8_t initData4[] = {0x5b, 0x03, 0x08, 0x10, 0x01, 0x5d};
        uint8_t initData5[] = {0x5b, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x5d};
        uint8_t initData6[] = {0x5b, 0x02, 0x02, 0x02, 0x5d};
        uint8_t initData7[] = {0x5b, 0x02, 0x03, 0x04, 0x5d};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    } else {
        uint8_t initData1[] = {0x5b, 0x01, 0xf0, 0x5d};
        uint8_t initData2[] = {0x5b, 0x04, 0x00, 0x13, 0x4f, 0x4b, 0x5d};
        uint8_t initData3[] = {0x5b, 0x02, 0x03, 0x01, 0x5d};
        uint8_t initData4[] = {0x5b, 0x04, 0x00, 0x14, 0x4f, 0x4b, 0x5d};
        uint8_t initData5[] = {0x5b, 0x06, 0x07, 0x01, 0x23, 0x00, 0x9b, 0x43, 0x5d};
        uint8_t initData6[] = {0x5b, 0x03, 0x08, 0x10, 0x01, 0x5d};
        uint8_t initData7[] = {0x5b, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x5d};
        uint8_t initData8[] = {0x5b, 0x02, 0x02, 0x02, 0x5d};
        uint8_t initData9[] = {0x5b, 0x04, 0x00, 0x09, 0x4f, 0x4b, 0x5d};
        uint8_t initData10[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, true);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, true);
    }

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void solebike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
    QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &solebike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &solebike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &solebike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &solebike::descriptorWritten);

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
                // connect(virtualBike,&virtualbike::debug ,this,&solebike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &solebike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void solebike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void solebike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void solebike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &solebike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void solebike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("solebike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void solebike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("solebike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void solebike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';
    if (device.name().toUpper().startsWith(QStringLiteral("R92"))) {
        qDebug() << QStringLiteral("R92 found!");
        r92 = true;
    }

    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &solebike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &solebike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &solebike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &solebike::controllerStateChanged);

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
}

bool solebike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t solebike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

void solebike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
