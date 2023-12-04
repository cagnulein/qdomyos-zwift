#include "elitebike.h"
#include "virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include <chrono>

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

using namespace std::chrono_literals;

elitebike::elitebike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                   double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &elitebike::update);
    refresh->start(200ms);
}

void elitebike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                   bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if(!gattFTMSService) {
        qDebug() << QStringLiteral("gattFTMSService is null!");
        return;
    }

    if (wait_for_response) {
        connect(gattFTMSService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattFTMSService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharControlPointId.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer,
                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void elitebike::init() {
    if (initDone)
        return;

    initDone = true;
    initRequest = false;
}

void elitebike::forcePower(int16_t requestPower) {
    uint8_t write[] = {FTMS_SET_TARGET_POWER, 0x00, 0x00};

    write[1] = ((uint16_t)requestPower) & 0xFF;
    write[2] = ((uint16_t)requestPower) >> 8;

    writeCharacteristic(write, sizeof(write), QStringLiteral("forcePower ") + QString::number(requestPower));

    powerForced = true;
}

void elitebike::forceResistance(resistance_t requestResistance) {

    uint8_t write[] = {0x01, 0x00};
    write[1] = ((uint8_t)(requestResistance * 10));
    writeCharacteristic(write, sizeof(write),
                        QStringLiteral("forceResistance ") + QString::number(requestResistance));
}

void elitebike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState //&&
                                                                           // gattCommunicationChannelService &&
                                                                           // gattWriteCharacteristic.isValid() &&
                                                                           // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {
        update_metrics(false, watts());

               // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > 100) {
                requestResistance = 100;
            } // TODO, use the bluetooth value
            else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestPower != -1) {
            qDebug() << QStringLiteral("writing power") << requestPower;
            init();
            forcePower(requestPower);
            requestPower = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

                   // btinit();

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

void elitebike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void elitebike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    bool heart = false;

    qDebug() << characteristic.uuid() << QStringLiteral(" << ") << newValue.toHex(' ');

    lastPacket = newValue;

    if (characteristic.uuid() == QBluetoothUuid::CyclingPowerMeasurement) {
        lastPacket = newValue;

        uint16_t flags = (((uint16_t)((uint8_t)newValue.at(1)) << 8) | (uint16_t)((uint8_t)newValue.at(0)));
        bool cadence_present = false;
        bool wheel_revs = false;
        bool crank_rev_present = false;
        uint16_t time_division = 1024;
        uint8_t index = 4;

        if (newValue.length() > 3) {
            m_watt = (((uint16_t)((uint8_t)newValue.at(3)) << 8) | (uint16_t)((uint8_t)newValue.at(2)));
        }

        emit powerChanged(m_watt.value());
        emit debug(QStringLiteral("Current watt: ") + QString::number(m_watt.value()));

        if ((flags & 0x1) == 0x01) // Pedal Power Balance Present
        {
            index += 1;
        }
        if ((flags & 0x2) == 0x02) // Pedal Power Balance Reference
        {
        }
        if ((flags & 0x4) == 0x04) // Accumulated Torque Present
        {
            index += 2;
        }
        if ((flags & 0x8) == 0x08) // Accumulated Torque Source
        {
        }

        if ((flags & 0x10) == 0x10) // Wheel Revolution Data Present
        {
            cadence_present = true;
            wheel_revs = true;
        }

        if ((flags & 0x20) == 0x20) // Crank Revolution Data Present
        {
            cadence_present = true;
            crank_rev_present = true;
        }

        if (cadence_present) {
            if (wheel_revs && !crank_rev_present) {
                time_division = 2048;
                CrankRevs =
                    (((uint32_t)((uint8_t)newValue.at(index + 3)) << 24) |
                     ((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                     ((uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)));
                index += 4;

                LastCrankEventTime =
                    (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));

                index += 2; // wheel event time

            } else if (wheel_revs && crank_rev_present) {
                index += 4; // wheel revs
                index += 2; // wheel event time
            }

            if (crank_rev_present) {
                CrankRevs =
                    (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
                index += 2;

                LastCrankEventTime =
                    (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
                index += 2;
            }

            int16_t deltaT = LastCrankEventTime - oldLastCrankEventTime;
            if (deltaT < 0) {
                deltaT = LastCrankEventTime + time_division - oldLastCrankEventTime;
            }

            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                if (CrankRevs != oldCrankRevs && deltaT) {
                    double cadence = ((CrankRevs - oldCrankRevs) / deltaT) * time_division * 60;
                    if (!crank_rev_present)
                        cadence =
                            cadence /
                            2; // I really don't like this, there is no relationship between wheel rev and crank rev
                    if (cadence >= 0) {
                        Cadence = cadence;
                    }
                    lastGoodCadence = QDateTime::currentDateTime();
                } else if (lastGoodCadence.msecsTo(QDateTime::currentDateTime()) > 2000) {
                    Cadence = 0;
                }
            }

            qDebug() << QStringLiteral("Current Cadence: ") << Cadence.value() << CrankRevs << oldCrankRevs << deltaT
                     << time_division << LastCrankEventTime << oldLastCrankEventTime;

            oldLastCrankEventTime = LastCrankEventTime;
            oldCrankRevs = CrankRevs;

            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = Cadence.value() * settings
                                              .value(QZSettings::cadence_sensor_speed_ratio,
                                                     QZSettings::default_cadence_sensor_speed_ratio)
                                              .toDouble();
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

                   // if we change this, also change the wattsFromResistance function. We can create a standard function in
                   // order to have all the costants in one place (I WANT MORE TIME!!!)
            double ac = 0.01243107769;
            double bc = 1.145964912;
            double cc = -23.50977444;

            double ar = 0.1469553975;
            double br = -5.841344538;
            double cr = 97.62165482;

            double res =
                (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                           (cr - (m_watt.value() * 132.0 /
                                                  (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                   br) /
                  (2.0 * ar)) *
                 settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();

            if (isnan(res)) {
                if (Cadence.value() > 0) {
                    // let's keep the last good value
                } else {
                    m_pelotonResistance = 0;
                }
            } else {
                m_pelotonResistance = res;
            }

            qDebug() << QStringLiteral("Current Peloton Resistance: ") + QString::number(m_pelotonResistance.value());

            if (settings.value(QZSettings::schwinn_bike_resistance, QZSettings::default_schwinn_bike_resistance)
                    .toBool())
                Resistance = pelotonToBikeResistance(m_pelotonResistance.value());
            else
                Resistance = m_pelotonResistance;
            emit resistanceRead(Resistance.value());
            qDebug() << QStringLiteral("Current Resistance Calculated: ") + QString::number(Resistance.value());

            if (watts())
                KCal +=
                    ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                    QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight
                                                                      // in kg * 3.5) / 200 ) / 60
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
        }
    }

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) &&
        (!heart || Heart.value() == 0 || disable_hr_frommachinery)) {
        update_hr_from_external();
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

    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void elitebike::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    if (state != QLowEnergyService::ServiceState::ServiceDiscovered) {
        qDebug() << QStringLiteral("ignoring this state");
        return;
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &elitebike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &elitebike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &elitebike::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &elitebike::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &elitebike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &elitebike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                QBluetoothUuid _gattWriteCharControlPointId(QStringLiteral("347b0010-7635-408b-8918-8ff3949ce592"));
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("FTMS service and Control Point found");
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                }
            }
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
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&elitebike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &elitebike::changeInclination);
                connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &elitebike::ftmsCharacteristicChanged);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void elitebike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    if (!autoResistance()) {
        qDebug() << "ignoring routing FTMS packet to the bike from virtualbike because of auto resistance OFF"
                 << characteristic.uuid() << newValue.toHex(' ');
        return;
    }

    QByteArray b = newValue;
    if (gattWriteCharControlPointId.isValid()) {
        qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');

               // handling gears
        if (b.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) {
            qDebug() << "applying gears mod" << m_gears;
            int16_t slope = (((uint8_t)b.at(3)) + (b.at(4) << 8));
            if (m_gears != 0) {
                slope += (m_gears * 50);
                b[3] = slope & 0xFF;
                b[4] = slope >> 8;
            }
        }

        if (writeBuffer) {
            delete writeBuffer;
        }
        writeBuffer = new QByteArray(b);

        gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);
    }
}

void elitebike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void elitebike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void elitebike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void elitebike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void elitebike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    initRequest = false;
    auto services_list = m_control->services();
    QBluetoothUuid ftmsService((quint16)0x1826);
    bool JK_fitness_577 = bluetoothDevice.name().toUpper().startsWith("DHZ-");
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        if ((JK_fitness_577 && s == ftmsService) || !JK_fitness_577) {
            gattCommunicationChannelService.append(m_control->createServiceObject(s));
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &elitebike::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails();
        }
    }
}

void elitebike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("elitebike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void elitebike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("elitebike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

resistance_t elitebike::pelotonToBikeResistance(int pelotonResistance) {
    return (pelotonResistance * max_resistance) / 100;
}

void elitebike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        if (bluetoothDevice.name().toUpper().startsWith("SUITO")) {
            qDebug() << QStringLiteral("SUITO found");
            max_resistance = 16;
        } else if ((bluetoothDevice.name().toUpper().startsWith("MAGNUS "))) {
            qDebug() << QStringLiteral("MAGNUS found");
            resistance_lvl_mode = true;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &elitebike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &elitebike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &elitebike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &elitebike::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
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
            emit disconnected();
        });

               // Connect
        m_control->connectToDevice();
        return;
    }
}

bool elitebike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t elitebike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void elitebike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
