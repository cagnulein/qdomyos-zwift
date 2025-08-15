#include "apexbike.h"
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

apexbike::apexbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                   double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &apexbike::update);
    refresh->start(200ms);
}

void apexbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                            QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                        QStringLiteral(" // ") + info;
    }

    loop.exec();
}

void apexbike::forceResistance(resistance_t requestResistance) {}

void apexbike::sendPoll() {
    uint8_t noOpData[] = {0xaa, 0x55, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xc3, 0x3c};

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, false);
}

void apexbike::update() {
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

        // sending poll every 5 seconds
        if (sec1Update++ >= (5000 / refresh->interval())) {
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

void apexbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void apexbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

    if (newValue.length() == 10 && newValue.at(2) == 0x31) {
        // Invert resistance: bike resistance 1-32 maps to app display 32-1
        uint8_t rawResistance = newValue.at(5);
        Resistance = 33 - rawResistance;  // Invert: 1->32, 32->1
        emit resistanceRead(Resistance.value());
        m_pelotonResistance = Resistance.value();

        qDebug() << QStringLiteral("Raw resistance: ") + QString::number(rawResistance) + QStringLiteral(", Inverted resistance: ") + QString::number(Resistance.value());
    }

    if (newValue.length() != 10 || newValue.at(2) != 0x31) {
        return;
    }

    uint16_t distanceData = (newValue.at(7) << 8) | ((uint8_t)newValue.at(8));
    double distance = ((double)distanceData);

    if(distance != lastDistance) {
        if(lastDistance != 0) {
            double deltaDistance = distance - lastDistance;
            double deltaTime = fabs(now.msecsTo(lastTS));
            double timeHours = deltaTime / (1000.0 * 60.0 * 60.0);
            double k = 0.005333;
            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = (deltaDistance *k) / timeHours; // Speed in distance units per hour
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = Speed.value() / 0.37497622;
            }
        }
        lastDistance = distance;
        lastTS = now;
        qDebug() << "lastDistance" << lastDistance << "lastTS" << lastTS;
    } else {
        // Check if speed and cadence should be reset due to timeout (2 seconds)
        if (lastTS.msecsTo(now) > 2000) {
            if (Speed.value() > 0) {
                Speed = 0;
                qDebug() << "Speed reset to 0 due to timeout";
            }
            if (Cadence.value() > 0) {
                Cadence = 0;
                qDebug() << "Cadence reset to 0 due to timeout";
            }
            lastTS = now;
        }
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

void apexbike::btinit() {
    sendPoll();
    QThread::msleep(2000);
    sendPoll();
    QThread::msleep(2000);
    uint8_t initData1[] = {0xeb, 0x50, 0x51, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2};
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    QThread::msleep(500);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    QThread::msleep(500);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    QThread::msleep(500);

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void apexbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

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
                &apexbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &apexbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &apexbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &apexbike::descriptorWritten);

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
                // connect(virtualBike,&virtualbike::debug ,this,&apexbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &apexbike::changeInclination);

                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
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

void apexbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void apexbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void apexbike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &apexbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void apexbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("apexbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void apexbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("apexbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void apexbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &apexbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &apexbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &apexbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &apexbike::controllerStateChanged);

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

bool apexbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t apexbike::watts() {
    double resistance = Resistance.value();
    double cadence = Cadence.value();
    
    if (cadence <= 0 || resistance <= 0) {
        return 0;
    }
    
    // Comprehensive power table based on user data
    // Format: resistance level (1-19), RPM (10-150 in steps of 10), power (watts)
    static const int powerTable[19][15] = {
        // Resistance 1: RPM 10,20,30,40,50,60,70,80,90,100,110,120,130,140,150
        {12, 24, 36, 48, 61, 73, 85, 97, 109, 121, 133, 145, 157, 170, 182},
        // Resistance 2
        {24, 48, 73, 97, 121, 145, 170, 194, 218, 242, 266, 291, 315, 339, 363},
        // Resistance 3
        {36, 73, 109, 145, 182, 218, 254, 291, 327, 363, 400, 436, 472, 509, 545},
        // Resistance 4
        {48, 97, 145, 194, 242, 291, 339, 388, 436, 484, 533, 581, 630, 678, 727},
        // Resistance 5
        {61, 121, 182, 242, 303, 363, 424, 484, 545, 606, 666, 727, 787, 848, 908},
        // Resistance 6
        {73, 145, 218, 291, 363, 436, 509, 581, 654, 727, 799, 872, 945, 1017, 1090},
        // Resistance 7
        {85, 170, 254, 339, 424, 509, 593, 678, 763, 848, 933, 1017, 1102, 1187, 1272},
        // Resistance 8
        {97, 194, 291, 388, 484, 581, 678, 775, 872, 969, 1066, 1163, 1260, 1356, 1453},
        // Resistance 9
        {109, 218, 327, 436, 545, 654, 763, 872, 981, 1090, 1199, 1308, 1417, 1526, 1635},
        // Resistance 10
        {121, 242, 363, 484, 606, 727, 848, 969, 1090, 1211, 1332, 1453, 1574, 1696, 1817},
        // Resistance 11
        {133, 266, 400, 533, 666, 799, 933, 1066, 1199, 1332, 1465, 1599, 1732, 1865, 1998},
        // Resistance 12
        {145, 291, 436, 581, 727, 872, 1017, 1163, 1308, 1453, 1599, 1744, 1889, 2035, 2180},
        // Resistance 13
        {157, 315, 472, 630, 787, 945, 1102, 1260, 1417, 1574, 1732, 1889, 2047, 2204, 2362},
        // Resistance 14
        {170, 339, 509, 678, 848, 1017, 1187, 1356, 1526, 1696, 1865, 2035, 2204, 2374, 2543},
        // Resistance 15
        {182, 363, 545, 727, 908, 1090, 1272, 1453, 1635, 1817, 1998, 2180, 2362, 2543, 2725},
        // Resistance 16
        {194, 388, 581, 775, 969, 1163, 1356, 1550, 1744, 1938, 2132, 2325, 2519, 2713, 2907},
        // Resistance 17
        {206, 412, 618, 824, 1029, 1235, 1441, 1647, 1853, 2059, 2265, 2471, 2677, 2882, 3088},
        // Resistance 18
        {218, 436, 654, 872, 1090, 1308, 1526, 1744, 1962, 2180, 2398, 2616, 2834, 3052, 3270},
        // Resistance 19
        {230, 460, 690, 920, 1151, 1381, 1611, 1841, 2071, 2301, 2531, 2761, 2991, 3222, 3452}
    };
    
    // Clamp resistance to valid range (1-19)
    int res = qMax(1, qMin(19, (int)qRound(resistance)));
    
    // Convert to array index (0-18)
    int resIndex = res - 1;
    
    // RPM ranges from 10 to 150 in steps of 10
    // Find the two closest RPM values for interpolation
    double rpm = qMax(1.0, cadence); // Ensure RPM is at least 1
    
    if (rpm <= 10.0) {
        // Below minimum RPM, extrapolate from first data point
        double factor = rpm / 10.0;
        return (uint16_t)qMax(0.0, powerTable[resIndex][0] * factor);
    }
    
    if (rpm >= 150.0) {
        // Above maximum RPM, extrapolate from last data point
        double factor = rpm / 150.0;
        return (uint16_t)qMax(0.0, powerTable[resIndex][14] * factor);
    }
    
    // Find the two RPM values to interpolate between
    // RPM values are: 10, 20, 30, ..., 150 (indices 0-14)
    int lowerRpmIndex = ((int)rpm - 1) / 10;  // Convert RPM to array index
    if (lowerRpmIndex > 13) lowerRpmIndex = 13; // Ensure we don't go out of bounds
    
    int upperRpmIndex = lowerRpmIndex + 1;
    
    double lowerRpm = (lowerRpmIndex + 1) * 10.0;  // Convert index back to RPM
    double upperRpm = (upperRpmIndex + 1) * 10.0;
    
    int lowerPower = powerTable[resIndex][lowerRpmIndex];
    int upperPower = powerTable[resIndex][upperRpmIndex];
    
    // Linear interpolation between the two power values
    double ratio = (rpm - lowerRpm) / (upperRpm - lowerRpm);
    double interpolatedPower = lowerPower + ratio * (upperPower - lowerPower);
    
    return (uint16_t)qMax(0.0, interpolatedPower);
}

void apexbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
