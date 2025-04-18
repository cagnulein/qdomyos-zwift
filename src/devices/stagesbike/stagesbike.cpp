#include "stagesbike.h"
#include "virtualdevices/virtualbike.h"
#include "homeform.h"
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

using namespace std::chrono_literals;

stagesbike::stagesbike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &stagesbike::update);
    refresh->start(200ms);
}

void stagesbike::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                           uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!service) {
        qDebug() << "no gattCustomService available";
        return;
    }

    if (wait_for_response) {
        // TO FIX
        //connect(this, &stagesbike::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(2000, &loop, SLOT(quit())); // 6 seconds are important
    } else {
        connect(service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (characteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        service->writeCharacteristic(characteristic, *writeBuffer, QLowEnergyService::WriteWithoutResponse);
    } else {
        service->writeCharacteristic(characteristic, *writeBuffer);
    }

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}

void stagesbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        // required to the SS2K only one time
        Resistance = 0;
        emit resistanceRead(Resistance.value());
        initRequest = false;
        if(eliteService != nullptr) {
            uint8_t init1[] = {0x01, 0xad};
            writeCharacteristic(eliteService, eliteWriteCharacteristic, init1, sizeof(init1), "init", false, false);
            QThread::sleep(2);
            uint8_t init2[] = {0x45, 0x0a};
            writeCharacteristic(eliteService, eliteWriteCharacteristic2, init2, sizeof(init2), "init", false, false);
        }
    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState //&&
                                                                           // gattCommunicationChannelService &&
                                                                           // gattWriteCharacteristic.isValid() &&
                                                                           // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {
        QSettings settings;
        bool power_as_bike =
            settings.value(QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike).toBool();
        update_metrics(false, watts(), !power_as_bike);

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > 100) {
                requestResistance = 100;
            } else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value() || lastGearValue != gears()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                if(eliteService != nullptr) {
                    QByteArray a = setBrakeLevel((lastRawRequestedResistanceValue + gears()) / 100.0);
                    uint8_t b[20];
                    memcpy(b, a.constData(), a.length());
                    writeCharacteristic(eliteService, eliteWriteCharacteristic, b, a.length(), "forceResistance", false, false);
                }
            }
            requestResistance = -1;
        } else if(requestPower != -1) {
            QByteArray a = setTargetPower(requestPower);
            uint8_t b[20];
            memcpy(b, a.constData(), a.length());
            writeCharacteristic(eliteService, eliteWriteCharacteristic, b, a.length(), "forcePower", false, false);
            requestPower = -1;
        }

        lastGearValue = gears();

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

void stagesbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
    if(gatt == QBluetoothUuid((quint16)0x1826)) {
        QSettings settings;
        settings.setValue(QZSettings::ftms_bike, bluetoothDevice.name());
        qDebug() << "forcing FTMS bike since it has FTMS";
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("FTMS bike found, restart the app to apply the change!");
    }
}

resistance_t stagesbike::pelotonToBikeResistance(int pelotonResistance) {
    QSettings settings;
    bool schwinn_bike_resistance_v2 =
        settings.value(QZSettings::schwinn_bike_resistance_v2, QZSettings::default_schwinn_bike_resistance_v2).toBool();
    if (!schwinn_bike_resistance_v2) {
        if (pelotonResistance > 54)
            return pelotonResistance;
        if (pelotonResistance < 26)
            return pelotonResistance / 5;

        // y = 0,04x2 - 1,32x + 11,8
        return ((0.04 * pow(pelotonResistance, 2)) - (1.32 * pelotonResistance) + 11.8);
    } else {
        if (pelotonResistance > 20)
            return (((double)pelotonResistance - 20.0) * 1.25);
        else
            return 1;
    }
}

uint16_t stagesbike::wattsFromResistance(double resistance) {
    QSettings settings;

    double ac = 0.01243107769;
    double bc = 1.145964912;
    double cc = -23.50977444;

    double ar = 0.1469553975;
    double br = -5.841344538;
    double cr = 97.62165482;

    for (uint16_t i = 1; i < 2000; i += 5) {
        double res =
            (((sqrt(pow(br, 2.0) -
                    4.0 * ar *
                        (cr - ((double)i * 132.0 / (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
               br) /
              (2.0 * ar)) *
             settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
            settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();

        if (!isnan(res) && res >= resistance) {
            return i;
        }
    }

    return 0;
}

void stagesbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    if (characteristic.uuid() == QBluetoothUuid::CharacteristicType::CyclingPowerMeasurement) {
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
                    if (cadence >= 0 && cadence < 256) {
                        Cadence = cadence;
                    }
                    lastGoodCadence = now;
                } else if (lastGoodCadence.msecsTo(now) > 2000) {
                    Cadence = 0;
                }
            }

            emit cadenceChanged(Cadence.value());

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
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
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

            if (ResistanceFromFTMSAccessoryLastTime == 0) {
                if (settings.value(QZSettings::schwinn_bike_resistance, QZSettings::default_schwinn_bike_resistance)
                        .toBool())
                    Resistance = pelotonToBikeResistance(m_pelotonResistance.value());
                else
                    Resistance = m_pelotonResistance;
                emit resistanceRead(Resistance.value());
                qDebug() << QStringLiteral("Current Resistance Calculated: ") + QString::number(Resistance.value());
            } else {
                Resistance = ResistanceFromFTMSAccessory.value();
            }

            if (watts())
                KCal +=
                    ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                    now)))); //(( (0.048* Output in watts +1.19) * body weight
                                                                      // in kg * 3.5) / 200 ) / 60
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
        }
    }

    if (!noVirtualDevice) {
#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
            Heart = (uint8_t)KeepAwakeHelper::heart();
            debug("Current Heart: " + QString::number(Heart.value()));
        } else
#endif
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                update_hr_from_external();
        }
    }

    lastRefreshCharacteristicChanged = now;

    if (!noVirtualDevice) {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence =
            settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif
#endif
    }

    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void stagesbike::stateChanged(QLowEnergyService::ServiceState state) {                                                   
    QBluetoothUuid _EliteServiceId(QStringLiteral("347b0001-7635-408b-8918-8ff3949ce592"));
    QBluetoothUuid _EliteWriteId(QStringLiteral("347b0010-7635-408b-8918-8ff3949ce592"));
    QBluetoothUuid _EliteWrite2Id(QStringLiteral("347b0018-7635-408b-8918-8ff3949ce592"));
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::RemoteServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &stagesbike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &stagesbike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &stagesbike::characteristicRead);
            connect(
                s, &QLowEnergyService::errorOccurred,
                this, &stagesbike::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &stagesbike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &stagesbike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
                }

                if(s->serviceUuid() == _EliteServiceId) {                    
                    if(c.uuid() == _EliteWriteId) {
                        qDebug() << QStringLiteral("found elite write characteristic");
                        eliteService = s;
                        eliteWriteCharacteristic = c;
                    } else if(c.uuid() == _EliteWrite2Id) {
                        qDebug() << QStringLiteral("found elite write characteristic2");
                        eliteService = s;
                        eliteWriteCharacteristic2 = c;
                    }
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }
            }
        }
    }

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice() && !noVirtualDevice
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
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
            // connect(virtualBike,&virtualbike::debug ,this,&stagesbike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &stagesbike::inclinationChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void stagesbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void stagesbike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void stagesbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void stagesbike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void stagesbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &stagesbike::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void stagesbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("stagesbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void stagesbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("stagesbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void stagesbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &stagesbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &stagesbike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &stagesbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &stagesbike::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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

bool stagesbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}


uint16_t stagesbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void stagesbike::resistanceFromFTMSAccessory(resistance_t res) {
    ResistanceFromFTMSAccessory = res;
    ResistanceFromFTMSAccessoryLastTime = QDateTime::currentMSecsSinceEpoch();
    qDebug() << QStringLiteral("resistanceFromFTMSAccessory") << res;
}

void stagesbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

// Elite Methods
QByteArray stagesbike::setTargetPower(quint16 watts)
{
    quint16 clamped = qMin(watts, static_cast<quint16>(4000));
    QByteArray result;
    result.append(static_cast<char>(0x00));
    result.append(static_cast<char>(clamped & 0xFF));
    result.append(static_cast<char>((clamped >> 8) & 0xFF));
    return result;
}

QByteArray stagesbike::setBrakeLevel(double level)
{
    double clamped = qBound(0.0, level, 1.0);
    quint8 normalized = static_cast<quint8>(round(clamped * 200));
    QByteArray result;
    result.append(static_cast<char>(0x01));
    result.append(static_cast<char>(normalized));
    return result;
}

QByteArray stagesbike::setSimulationMode(double grade, double crr, double wrc,
                                           double windSpeedKPH, double draftingFactor)
{
    quint16 gradeN = static_cast<quint16>(((grade * 100) + 200) * 100);
    quint8 crrN = static_cast<quint8>(static_cast<int>(crr / 0.00005) & 0xFF);
    quint8 wrcN = static_cast<quint8>(static_cast<int>(wrc / 0.01) & 0xFF);
    quint8 windSpeed = static_cast<quint8>(qBound(-127.0, windSpeedKPH, 128.0) + 127);
    quint8 draftN = static_cast<quint8>(static_cast<int>(draftingFactor / 0.01) & 0xFF);

    QByteArray result;
    result.append(static_cast<char>(0x02));
    result.append(static_cast<char>(gradeN & 0xFF));
    result.append(static_cast<char>((gradeN >> 8) & 0xFF));
    result.append(static_cast<char>(crrN));
    result.append(static_cast<char>(wrcN));
    result.append(static_cast<char>(windSpeed));
    result.append(static_cast<char>(draftN));
    return result;
}
