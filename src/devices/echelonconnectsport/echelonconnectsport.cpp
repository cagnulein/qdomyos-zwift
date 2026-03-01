#include "echelonconnectsport.h"
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

echelonconnectsport::echelonconnectsport(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
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
    connect(refresh, &QTimer::timeout, this, &echelonconnectsport::update);
    refresh->start(200ms);
}

void echelonconnectsport::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                              bool wait_for_response) {
    QSettings settings;
    bool useNativeIOS = false;

#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
#endif

    QEventLoop loop;
    QTimer timeout;

    if (!useNativeIOS) {
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
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

#ifdef Q_OS_IOS
    if (useNativeIOS) {
#ifndef IO_UNDER_QT
        iOS_echelonConnectSport->echelonConnectSport_WriteCharacteristic((unsigned char*)writeBuffer->data(), data_len);
#endif
    } else
#endif
    {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                        QStringLiteral(" // ") + info;
    }

    if (!useNativeIOS) {
        loop.exec();
    }
}

void echelonconnectsport::forceResistance(resistance_t requestResistance) {
    uint8_t noOpData[] = {0xf0, 0xb1, 0x01, 0x00, 0x00};

    noOpData[3] = requestResistance;

    for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
        noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("force resistance"), false, true);
}

void echelonconnectsport::sendPoll() {
    uint8_t noOpData[] = {0xf0, 0xa0, 0x01, 0x00, 0x00};

    noOpData[3] = counterPoll;

    for (uint8_t i = 0; i < sizeof(noOpData) - 1; i++) {
        noOpData[4] += noOpData[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);

    counterPoll++;
    if (!counterPoll)
        counterPoll = 1;
}

void echelonconnectsport::update() {

    if (!m_control)
        return;

    QSettings settings;
    bool useNativeIOS = false;

#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
#endif

    if (!useNativeIOS && m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if ((useNativeIOS ||
                (bluetoothDevice.isValid() &&
                 m_control->state() == QLowEnergyController::DiscoveredState &&
                 gattCommunicationChannelService &&
                 gattWriteCharacteristic.isValid() &&
                 gattNotify1Characteristic.isValid() &&
                 gattNotify2Characteristic.isValid())) &&
               initDone) {
        update_metrics(true, watts());

        // Continuous ERG mode support - recalculate resistance as cadence changes when using power zone tiles
        if (RequestedPower.value() > 0) {
            changePower(RequestedPower.value());
        }

               // sending poll every 2 seconds
        if (sec1Update++ >= (2000 / refresh->interval())) {
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

void echelonconnectsport::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

resistance_t echelonconnectsport::pelotonToBikeResistance(int pelotonResistance) {
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

resistance_t echelonconnectsport::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    if (Cadence.value() == 0)
        return 1;

    for (resistance_t i = 1; i < max_resistance; i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest") << wattsFromResistance(i)
                     << wattsFromResistance(i + 1) << power;
            return i;
        }
    }
    if (power < wattsFromResistance(1))
        return 1;
    else
        return max_resistance;
}

double echelonconnectsport::bikeResistanceToPeloton(double resistance) {
    QSettings settings;
    // 0,0097x3 - 0,4972x2 + 10,126x - 37,08
    double p = ((pow(resistance, 3) * 0.0097) - (0.4972 * pow(resistance, 2)) + (10.126 * resistance) - 37.08);
    if (p < 0) {
        p = 0;
    }
    return (p * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
           settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
}

void echelonconnectsport::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool useNativeIOS = false;

#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
#endif

    qDebug() << " << " + newValue.toHex(' ');

    lastPacket = newValue;

           // resistance value is in another frame
    if (newValue.length() == 5 && ((unsigned char)newValue.at(0)) == 0xf0 && ((unsigned char)newValue.at(1)) == 0xd2) {
        resistance_t res = newValue.at(3);
        if (settings.value(QZSettings::gears_from_bike, QZSettings::default_gears_from_bike).toBool()) {
            qDebug() << QStringLiteral("gears_from_bike") << res << Resistance.value() << gears()
                     << lastRawRequestedResistanceValue << lastRequestedResistance().value();
            if (
                 // if the resistance is different from the previous one
                res != qRound(Resistance.value()) &&
                // and the last target resistance is different from the current one or there is no any pending last
                // requested resistance
                ((lastRequestedResistance().value() != res && lastRequestedResistance().value() != 0) ||
                 lastRawRequestedResistanceValue == -1) &&
                // and the difference between the 2 resistances are less than 6
                qRound(Resistance.value()) > 1 && qAbs(res - qRound(Resistance.value())) < 6) {

                int8_t g = gears();
                g += (res - qRound(Resistance.value()));
                qDebug() << QStringLiteral("gears_from_bike APPLIED") << gears() << g;
                resistance_t savedRawValue = lastRawRequestedResistanceValue;
                lastRawRequestedResistanceValue = -1; // temporarily prevent setGears from re-applying resistance
                setGears(g);
                lastRawRequestedResistanceValue = savedRawValue; // restore for future checks
            }
        }
        Resistance = res;
        emit resistanceRead(Resistance.value());
        m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());

        qDebug() << QStringLiteral("Current resistance: ") + QString::number(Resistance.value());
        return;
    }

    if (newValue.length() != 13) {
        return;
    }

    /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
        return;*/

    double distance = GetDistanceFromPacket(newValue);

    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = ((uint8_t)newValue.at(10));
    }
    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = 0.37497622 * ((double)Cadence.value());
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }
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

    if (!useNativeIOS && m_control && m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

QTime echelonconnectsport::GetElapsedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(3) << 8) | packet.at(4);
    QTime t(0, convertedData / 60, convertedData % 60);
    return t;
}

double echelonconnectsport::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(7) << 8) | packet.at(8);
    double data = ((double)convertedData) / 100.0f;
    return data;
}

void echelonconnectsport::btinit() {
    uint8_t initData1[] = {0xf0, 0xa1, 0x00, 0x91};
    uint8_t initData2[] = {0xf0, 0xa3, 0x00, 0x93};
    uint8_t initData3[] = {0xf0, 0xb0, 0x01, 0x01, 0xa2};
    // uint8_t initData4[] = { 0xf0, 0x60, 0x00, 0x50 }; // get sleep command

    // useless i guess
    // writeCharacteristic(initData4, sizeof(initData4), "get sleep", false, true);

    // in the snoof log it repeats this frame 4 times, i will have to analyze the response to understand if 4 times are
    // enough
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void echelonconnectsport::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    bool useNativeIOS = false;

#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
#endif

    if (!useNativeIOS) {
        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("0bf669f2-45f2-11e7-9598-0800200c9a66"));
        QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("0bf669f3-45f2-11e7-9598-0800200c9a66"));
        QBluetoothUuid _gattNotify2CharacteristicId(QStringLiteral("0bf669f4-45f2-11e7-9598-0800200c9a66"));

        QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
        qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

        if (state == QLowEnergyService::RemoteServiceDiscovered) {
            // qDebug() << gattCommunicationChannelService->characteristics();

            gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
            gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
            gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
            Q_ASSERT(gattWriteCharacteristic.isValid());
            Q_ASSERT(gattNotify1Characteristic.isValid());
            Q_ASSERT(gattNotify2Characteristic.isValid());

            // establish hook into notifications
            connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                    &echelonconnectsport::characteristicChanged);
            connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                    &echelonconnectsport::characteristicWritten);
            connect(gattCommunicationChannelService,
                    &QLowEnergyService::errorOccurred,
                    this, &echelonconnectsport::errorService);
            connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                    &echelonconnectsport::descriptorWritten);
        }
    }

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
                    // connect(virtualBike,&virtualbike::debug ,this,&echelonconnectsport::debug);
                    connect(virtualBike, &virtualbike::changeInclination, this, &echelonconnectsport::changeInclination);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
                }
            }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************

    if (!useNativeIOS && state == QLowEnergyService::ServiceDiscovered) {
        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify2Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void echelonconnectsport::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void echelonconnectsport::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void echelonconnectsport::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QSettings settings;
    bool useNativeIOS = false;

#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
#endif

    if (!useNativeIOS) {
            QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("0bf669f1-45f2-11e7-9598-0800200c9a66"));

            gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
            connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
                    &echelonconnectsport::stateChanged);
            if(gattCommunicationChannelService != nullptr) {
                gattCommunicationChannelService->discoverDetails();
            } else {
                if(homeform::singleton())
                    homeform::singleton()->setToastRequested("Bluetooth Service Error! Restart the bike!");
                m_control->disconnectFromDevice();
            }
    }
}

void echelonconnectsport::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("echelonconnectsport::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void echelonconnectsport::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("echelonconnectsport::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void echelonconnectsport::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';

    QSettings settings;
    bool useNativeIOS = false;

#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
    if (useNativeIOS) {
#ifndef IO_UNDER_QT
        iOS_echelonConnectSport = new lockscreen();
        iOS_echelonConnectSport->echelonConnectSport(device.name().toStdString().c_str(), this);
        return;
#endif
    }
#endif

    if (device.name().startsWith(QStringLiteral("ECH"))) {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &echelonconnectsport::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &echelonconnectsport::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &echelonconnectsport::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &echelonconnectsport::controllerStateChanged);

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

bool echelonconnectsport::connected() {
    QSettings settings;
    bool useNativeIOS = false;

#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
    if (useNativeIOS) {
        return true;
    }
#endif

    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t echelonconnectsport::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return wattsFromResistance(Resistance.value());
}

uint16_t echelonconnectsport::wattsFromResistance(double resistance) {
    // https://github.com/cagnulein/qdomyos-zwift/issues/62#issuecomment-736913564
    /*if(currentCadence().value() < 90)
        return (uint16_t)((3.59 * exp(0.0217 * (double)(currentCadence().value()))) * exp(0.095 *
    (double)(currentResistance().value())) ); else return (uint16_t)((3.59 * exp(0.0217 *
    (double)(currentCadence().value()))) * exp(0.088 * (double)(currentResistance().value())) );*/

    QSettings settings;
    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        const double Epsilon = 4.94065645841247E-324;
        const int wattTableFirstDimension = 33;
        const int wattTableSecondDimension = 11;
        static const double wattTable[wattTableFirstDimension][wattTableSecondDimension] = {
            {Epsilon, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
            {Epsilon, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
            {Epsilon, 1.3, 3.0, 5.4, 10.4, 14.5, 18.5, 24.6, 27.6, 33.5, 49.5},
            {Epsilon, 1.5, 3.7, 6.7, 11.7, 15.9, 19.6, 26.1, 30.8, 35.2, 51.2},
            {Epsilon, 1.6, 4.7, 7.5, 13.7, 17.6, 22.6, 29.0, 36.9, 42.6, 57.2},
            {Epsilon, 1.8, 5.2, 8.0, 14.8, 19.1, 23.5, 32.5, 37.5, 50.8, 61.8},
            {Epsilon, 1.9, 5.7, 8.7, 15.6, 20.2, 25.5, 33.5, 39.6, 52.1, 65.3},
            {Epsilon, 2.0, 6.2, 9.5, 16.8, 21.8, 28.1, 37.0, 42.8, 57.8, 68.4},
            {Epsilon, 2.1, 6.8, 10.8, 18.2, 23.6, 29.5, 40.0, 47.6, 60.5, 72.1},
            {Epsilon, 2.2, 7.3, 11.5, 19.3, 26.3, 33.5, 45.3, 51.8, 66.7, 76.8},
            {Epsilon, 2.4, 7.9, 12.7, 20.8, 29.8, 37.6, 52.2, 56.2, 73.5, 83.6},
            {Epsilon, 2.6, 8.5, 13.5, 23.5, 33.6, 41.9, 55.1, 59.0, 78.6, 89.7},
            {Epsilon, 2.7, 9.1, 14.2, 25.6, 35.4, 45.3, 57.3, 62.8, 81.3, 95.0},
            {Epsilon, 2.9, 9.6, 16.8, 29.1, 37.5, 49.6, 62.5, 69.0, 84.7, 99.3},
            {Epsilon, 3.0, 10.0, 22.3, 31.2, 40.3, 51.8, 65.0, 70.0, 92.6, 108.2},
            {Epsilon, 3.2, 10.4, 24.0, 36.6, 42.5, 56.3, 74.0, 85.0, 98.2, 123.5},
            {Epsilon, 3.5, 10.9, 25.1, 38.5, 47.6, 65.4, 83.0, 93.0, 114.8, 136.8},
            {Epsilon, 3.7, 11.5, 26.0, 41.0, 53.2, 71.6, 90.0, 100.0, 121.7, 149.2},
            {Epsilon, 4.0, 12.1, 27.5, 43.6, 56.0, 82.3, 101.0, 113.6, 143.0, 162.8},
            {Epsilon, 4.2, 12.7, 29.7, 46.7, 64.2, 87.9, 109.2, 128.9, 154.0, 172.3},
            {Epsilon, 4.5, 13.7, 32.0, 50.0, 71.8, 95.6, 113.8, 135.6, 165.0, 185.0},
            {Epsilon, 4.7, 14.9, 34.5, 54.2, 77.0, 100.7, 127.0, 147.6, 180.0, 200.0},
            {Epsilon, 5.0, 15.8, 36.5, 58.3, 83.4, 110.1, 136.0, 168.1, 196.0, 213.5},
            {Epsilon, 5.6, 17.0, 39.5, 64.3, 88.8, 123.4, 154.0, 182.0, 210.0, 235.0},
            {Epsilon, 6.1, 18.2, 44.0, 70.7, 99.9, 133.3, 166.0, 198.0, 230.0, 253.5},
            {Epsilon, 6.8, 19.4, 49.0, 79.0, 108.8, 147.2, 185.0, 217.0, 255.2, 278.0},
            {Epsilon, 7.6, 22.0, 54.8, 88.0, 127.0, 167.0, 212.0, 244.0, 287.0, 305.0},
            {Epsilon, 8.7, 26.0, 62.0, 100.0, 145.0, 190.0, 242.0, 281.0, 315.1, 350.0},
            {Epsilon, 9.2, 30.0, 71.0, 114.4, 161.6, 215.1, 275.1, 317.0, 358.5, 390.0},
            {Epsilon, 9.8, 36.0, 82.5, 134.5, 195.3, 252.5, 313.7, 360.0, 420.3, 460.0},
            {Epsilon, 10.5, 43.0, 95.0, 157.1, 228.4, 300.1, 374.1, 403.8, 487.8, 540.0},
            {Epsilon, 12.5, 48.0, 99.3, 162.2, 232.9, 310.4, 400.3, 435.5, 530.5, 589.0},
            {Epsilon, 13.0, 53.0, 102.0, 170.3, 242.0, 320.0, 427.9, 475.2, 570.0, 625.0}};

        static const double wattTable_mgarcea[wattTableFirstDimension][wattTableSecondDimension] = {
            {Epsilon, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
            {Epsilon, 1.0, 2.2, 4.8, 9.5, 13.6, 16.7, 22.6, 26.3, 29.2, 47.0},
            {Epsilon, 1.3, 3.0, 5.4, 10.4, 14.5, 18.5, 24.6, 27.6, 33.5, 49.5},
            {Epsilon, 1.5, 3.7, 6.7, 11.7, 15.9, 19.6, 26.1, 30.8, 35.2, 51.2},
            {Epsilon, 1.6, 4.7, 7.5, 13.7, 17.6, 22.6, 29.0, 36.9, 42.6, 57.2},
            {Epsilon, 1.8, 5.2, 8.0, 14.8, 19.1, 23.5, 32.5, 37.5, 50.8, 61.8},
            {Epsilon, 1.9, 5.7, 8.7, 15.6, 20.2, 25.5, 33.5, 39.6, 52.1, 65.3},
            {Epsilon, 2.0, 6.2, 9.5, 16.8, 21.8, 28.1, 37.0, 42.8, 57.8, 68.4},
            {Epsilon, 2.1, 6.8, 10.8, 18.2, 23.6, 29.5, 40.0, 47.6, 60.5, 72.1},
            {Epsilon, 2.2, 7.3, 11.5, 19.3, 26.3, 33.5, 45.3, 51.8, 66.7, 76.8},
            {Epsilon, 2.4, 7.9, 12.7, 20.8, 15.6, 22.6, 29.6, 36.6, 43.5, 50.5},
            {Epsilon, 2.6, 8.5, 13.5, 23.5, 19.7, 26.9, 34.1, 41.4, 48.6, 55.8},
            {Epsilon, 2.7, 9.1, 14.2, 25.6, 23.3, 30.3, 37.3, 44.3, 51.3, 58.3},
            {Epsilon, 2.9, 9.6, 16.8, 29.1, 27.9, 34.6, 41.3, 48.0, 54.7, 61.4},
            {Epsilon, 3.0, 10.0, 22.3, 31.2, 28.2, 36.8, 45.4, 54.0, 62.6, 71.2},
            {Epsilon, 3.2, 10.4, 24.0, 36.6, 28.5, 40.4, 53.0, 65.6, 78.2, 90.8},
            {Epsilon, 3.5, 10.9, 25.1, 38.5, 29.2, 44.1, 59.0, 73.9, 88.8, 103.7},
            {Epsilon, 3.7, 11.5, 26.0, 41.0, 36.3, 47.8, 70.0, 86.9, 103.7, 120.6},
            {Epsilon, 4.0, 12.1, 27.5, 43.6, 43.4, 49.5, 71.0, 90.5, 111.0, 131.5},
            {Epsilon, 4.2, 12.7, 29.7, 46.7, 50.5, 58.0, 82.0, 104.0, 126.0, 150.0},
            {Epsilon, 4.5, 13.7, 32.0, 50.0, 54.6, 77.7, 100.8, 123.9, 147.0, 170.1},
            {Epsilon, 4.7, 14.9, 34.5, 54.2, 61.0, 87.5, 114.0, 140.5, 167.0, 193.5},
            {Epsilon, 5.0, 15.8, 36.5, 58.3, 63.0, 94.5, 126.0, 157.5, 189.0, 220.5},
            {Epsilon, 5.6, 17.0, 39.5, 64.3, 77.0, 111.0, 145.0, 179.0, 213.0, 247.0},
            {Epsilon, 6.1, 18.2, 44.0, 70.7, 88.0, 125.5, 163.0, 200.5, 238.0, 275.5},
            {Epsilon, 6.8, 19.4, 49.0, 79.0, 99.8, 142.4, 185.0, 227.6, 270.2, 312.8},
            {Epsilon, 7.6, 22.0, 54.8, 88.0, 101.0, 155.0, 209.0, 263.0, 317.0, 371.0},
            {Epsilon, 8.7, 26.0, 62.0, 100.0, 118.9, 178.0, 237.0, 296.1, 355.1, 414.2},
            {Epsilon, 9.2, 30.0, 71.0, 114.4, 131.7, 195.9, 260.1, 324.3, 388.5, 452.7},
            {Epsilon, 9.8, 36.0, 82.5, 134.5, 144.1, 215.9, 287.7, 359.5, 431.3, 503.1},
            {Epsilon, 10.5, 43.0, 95.0, 157.1, 158.4, 238.3, 318.1, 398.0, 477.8, 557.7},
            {Epsilon, 12.5, 48.0, 99.3, 162.2, 164.1, 257.2, 350.3, 443.4, 536.5, 629.6},
            {Epsilon, 13.0, 53.0, 102.0, 170.3, 194.8, 292.4, 389.9, 487.5, 585.0, 682.6}};

        int level = resistance;
        if (level < 0) {
            level = 0;
        }
        if (level >= wattTableFirstDimension) {
            level = wattTableFirstDimension - 1;
        }
        const double *watts_of_level;
        QSettings settings;
        if (!settings.value(QZSettings::echelon_watttable, QZSettings::default_echelon_watttable)
                 .toString()
                 .compare("mgarcea"))
            watts_of_level = wattTable_mgarcea[level];
        else
            watts_of_level = wattTable[level];
        int watt_setp = (Cadence.value() / 10.0);
        if (watt_setp >= 10) {
            return (((double)Cadence.value()) / 100.0) * watts_of_level[wattTableSecondDimension - 1];
        }
        double watt_base = watts_of_level[watt_setp];
        return (((watts_of_level[watt_setp + 1] - watt_base) / 10.0) * ((double)(((int)(Cadence.value())) % 10))) +
               watt_base;
    } else {
        return _ergTable.estimateWattage(Cadence.value(), resistance);
    }
}

void echelonconnectsport::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    QSettings settings;
    bool useNativeIOS = false;
    
#ifdef Q_OS_IOS
    useNativeIOS = settings.value(QZSettings::ios_btdevice_native, QZSettings::default_ios_btdevice_native).toBool();
#endif
    
    if (state == QLowEnergyController::UnconnectedState
        && (m_control || useNativeIOS)
        ) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        if(!useNativeIOS)
            m_control->connectToDevice();

    }
}
