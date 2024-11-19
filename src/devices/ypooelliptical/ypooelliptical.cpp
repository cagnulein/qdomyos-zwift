#include "ypooelliptical.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualtreadmill.h"
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
#include <chrono>
#include "keepawakehelper.h"

using namespace std::chrono_literals;

ypooelliptical::ypooelliptical(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                               double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &ypooelliptical::update);
    refresh->start(200ms);

    // this bike doesn't send resistance, so I have to use the default value
    Resistance = default_resistance;
}

void ypooelliptical::writeCharacteristic(QLowEnergyCharacteristic* characteristic, QLowEnergyService *service, uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                         bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!service) {
        qDebug() << "gattCustomService nullptr";
        return;
    }

    if (wait_for_response) {
        connect(service, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(service, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (characteristic->properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        service->writeCharacteristic(*characteristic, *writeBuffer, QLowEnergyService::WriteWithoutResponse);
    } else {
        service->writeCharacteristic(*characteristic, *writeBuffer);
    }

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void ypooelliptical::forceInclination(double inclination) {
    uint8_t write[] = {FTMS_SET_TARGET_INCLINATION, 0x00, 0x00};
    write[1] = ((uint16_t)inclination * 10) & 0xFF;
    write[2] = ((uint16_t)inclination * 10) >> 8;
    writeCharacteristic(&gattFTMSWriteCharControlPointId, gattFTMSService, write, sizeof(write),
                        QStringLiteral("forceInclination ") + QString::number(inclination));
}

void ypooelliptical::forceResistance(resistance_t requestResistance) {

    if(E35 || SCH_590E || KETTLER) {
        uint8_t write[] = {FTMS_SET_TARGET_RESISTANCE_LEVEL, 0x00};
        write[1] = ((uint16_t)requestResistance * 10) & 0xFF;
        writeCharacteristic(&gattFTMSWriteCharControlPointId, gattFTMSService, write, sizeof(write),
                            QStringLiteral("forceResistance ") + QString::number(requestResistance));        
    } else {
        uint8_t write[] = {0x02, 0x44, 0x05, 0x01, 0x00, 0x40, 0x03};

        write[3] = (uint8_t)(requestResistance);
        write[5] = (uint8_t)(0x39 + requestResistance);

        writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, write, sizeof(write), QStringLiteral("forceResistance ") + QString::number(requestResistance));

        // this bike doesn't send resistance, so I have to use the value forced
        Resistance = requestResistance;
    }
}

void ypooelliptical::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    QSettings settings;
    bool iconsole_elliptical = settings.value(QZSettings::iconsole_elliptical, QZSettings::default_iconsole_elliptical).toBool();

    if (initRequest) {
        initRequest = false;
        if(E35 || SCH_590E || KETTLER) {
            uint8_t write[] = {FTMS_REQUEST_CONTROL};
            writeCharacteristic(&gattFTMSWriteCharControlPointId, gattFTMSService, write, sizeof(write), "requestControl", false, true);
        } else {
            uint8_t init1[] = {0x02, 0x42, 0x42, 0x03};
            uint8_t init2[] = {0x02, 0x41, 0x02, 0x43, 0x03};
            uint8_t init3[] = {0x02, 0x43, 0x01, 0x42, 0x03};
            uint8_t init4[] = {0x02, 0x44, 0x01, 0x45, 0x03};
            uint8_t init5[] = {0x02, 0x44, 0x05, 0x01, 0x00, 0x40, 0x03};

            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init1, sizeof(init1), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init2, sizeof(init2), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init3, sizeof(init3), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init1, sizeof(init1), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init4, sizeof(init4), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init3, sizeof(init3), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init5, sizeof(init5), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init1, sizeof(init1), QStringLiteral("init"), false, true);
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init5, sizeof(init5), QStringLiteral("init"), false, true);
        }
        initDone = true;
    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState //&&
                                                                           // gattCommunicationChannelService &&
                                                                           // gattWriteCharacteristic.isValid() &&
                                                                           // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {
        update_metrics(iconsole_elliptical, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        uint8_t init1[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t init3[] = {0x02, 0x43, 0x01, 0x42, 0x03};

        if (counterPoll == 0)
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init1, sizeof(init1), QStringLiteral("init"), false, true);
        else
            writeCharacteristic(&gattWriteCharControlPointId, gattCustomService, init3, sizeof(init3), QStringLiteral("init"), false, true);

        counterPoll++;
        if (counterPoll > 1)
            counterPoll = 0;

        if (requestResistance != -1) {
            if (requestResistance > max_resistance) {
                requestResistance = max_resistance;
            } else if(requestResistance < 1) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                auto virtualBike = dynamic_cast<virtualbike *>(this->VirtualDevice());
                if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike)) {
                    emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                    forceResistance(requestResistance);
                }
            }
            requestResistance = -1;
        }
        if (requestInclination != -100) {
            if(requestInclination < 1) {
                requestInclination = 1; // E35 min value
            }
            if (requestInclination != currentInclination().value()) {
                emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));
                forceInclination(requestInclination);
            }
            requestInclination = -100;
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

void ypooelliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void ypooelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newvalue) {
    QDateTime now = QDateTime::currentDateTime();
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    bool iconsole_elliptical =
        settings.value(QZSettings::iconsole_elliptical, QZSettings::default_iconsole_elliptical).toBool();

    qDebug() << characteristic.uuid() << QStringLiteral("<<") << newvalue.toHex(' ');

    if (characteristic.uuid() == QBluetoothUuid::HeartRate && newvalue.length() > 1) {
        Heart = (uint8_t)newvalue[1];
        emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
        return;
    }

    if(iconsole_elliptical && initDone == false)
        initRequest = true;

    union flags {
        struct {
            uint32_t moreData : 1;
            uint32_t avgSpeed : 1;
            uint32_t totDistance : 1;
            uint32_t stepCount : 1;
            uint32_t strideCount : 1;
            uint32_t elevationGain : 1;
            uint32_t rampAngle : 1;
            uint32_t resistanceLvl : 1;
            uint32_t instantPower : 1;
            uint32_t avgPower : 1;
            uint32_t expEnergy : 1;
            uint32_t heartRate : 1;
            uint32_t metabolicEq : 1;
            uint32_t elapsedTime : 1;
            uint32_t remainingTime : 1;
            uint32_t movementDirection : 1;
            uint32_t spare : 8;
        };

        uint32_t word_flags;
    };

    flags Flags;

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACE) && !iconsole_elliptical) {

        if(E35 == false && SCH_590E == false && KETTLER == false) {
            if (newvalue.length() == 18) {
                qDebug() << QStringLiteral("let's wait for the next piece of frame");
                lastPacket = newvalue;
                return;
            } else if (newvalue.length() == 17) {
                lastPacket.append(newvalue);
            } else {
                qDebug() << "packet not handled!!";
                return;
            }
        } else {
            lastPacket = newvalue;
        }

        int index = 0;
        Flags.word_flags = (lastPacket.at(2) << 16) | (lastPacket.at(1) << 8) | lastPacket.at(0);
        index += 3;

        if (!Flags.moreData) {
            if(E35 || SCH_590E || KETTLER) {
                Speed = ((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                                (uint16_t)((uint8_t)lastPacket.at(index)))) /
                        100.0;
                emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
            }
            index += 2;
        }

        // this particular device, seems to send the actual speed here
        if (Flags.avgSpeed) {
            // double avgSpeed;
            if(!E35 && !SCH_590E && !KETTLER) {
                Speed = ((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)lastPacket.at(index)))) /
                    100.0;
                emit debug(QStringLiteral("Current Average Speed: ") + QString::number(Speed.value()));
            }
            index += 2;            
        }

        if (Flags.totDistance) {
            if(!E35 && !SCH_590E && !KETTLER) {
                Distance = ((double)((((uint32_t)((uint8_t)lastPacket.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)lastPacket.at(index)))) /
                       1000.0;
            } else {
                Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
            }
            index += 3;
        } else {
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.stepCount) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                double divisor = 1.0;
                if(E35 || SCH_590E || KETTLER)
                    divisor = 2.0;
                Cadence = (((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)lastPacket.at(index))))) / divisor;
            }
            emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

            index += 2;
            index += 2;
        }

        if (Flags.strideCount) {
            index += 2;
        }

        if (Flags.elevationGain) {
            index += 2;
            index += 2;
        }

        if (Flags.rampAngle) {
            Inclination = (((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)lastPacket.at(index))))) / 10.0;
            emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));            
            index += 2;
            index += 2;
        }

        if(index + 1 >= lastPacket.length()) {
            qDebug() << "packet malformed" << index << lastPacket.length();
            return;
        }
        
        if (Flags.resistanceLvl) {
            Resistance = ((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)lastPacket.at(index))));
            // emit resistanceRead(Resistance.value());
            index += 2;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        } else {
            double ac = 0.01243107769;
            double bc = 1.145964912;
            double cc = -23.50977444;

            double ar = 0.1469553975;
            double br = -5.841344538;
            double cr = 97.62165482;

            if (Cadence.value() && m_watt.value()) {
                m_pelotonResistance =
                    (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                               (cr - (m_watt.value() * 132.0 /
                                                      (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                       br) /
                      (2.0 * ar)) *
                     settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
                Resistance = m_pelotonResistance;
                // emit resistanceRead(Resistance.value());
            }
        }

        if (Flags.instantPower) {
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                double divisor = 100.0; // i added this because this device seems to send it multiplied by 100

                if(E35 || SCH_590E || KETTLER)
                    divisor = 1.0;

                m_watt = ((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)lastPacket.at(index)))) /
                         divisor;
            }
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
            index += 2;
        }

        if (Flags.avgPower && lastPacket.length() > index + 1 && !E35 && !SCH_590E && !KETTLER) { // E35 has a bug about this
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)lastPacket.at(index))));
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
            index += 2;
        }

        if (Flags.expEnergy && lastPacket.length() > index + 1) {
            KCal = ((double)(((uint16_t)((uint8_t)lastPacket.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)lastPacket.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (watts())
                KCal += ((((0.048 * ((double)watts()) + 1.19) *
                           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                          200.0) /
                         (60000.0 /
                          ((double)lastRefreshCharacteristicChanged.msecsTo(
                              now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                // kg * 3.5) / 200 ) / 60
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && lastPacket.length() > index) {
                Heart = ((double)(((uint8_t)lastPacket.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
        }

        if (Flags.metabolicEq) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }
    } else if (iconsole_elliptical) {
        if (newvalue.length() == 15) {
            Speed = (double)((((uint8_t)newvalue.at(10)) << 8) | ((uint8_t)newvalue.at(9))) / 100.0;
            Cadence = newvalue.at(6);

            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

            if (watts())
                KCal += ((((0.048 * ((double)watts()) + 1.19) *
                           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                          200.0) /
                         (60000.0 /
                          ((double)lastRefreshCharacteristicChanged.msecsTo(
                              now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                // kg * 3.5) / 200 ) / 60

#ifdef Q_OS_ANDROID
            if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
                Heart = (uint8_t)KeepAwakeHelper::heart();
            else
#endif
            {

            }

            emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
            emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
            emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
            emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));
            emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
        }

        if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) &&
            (!Flags.heartRate || Heart.value() == 0 || disable_hr_frommachinery)) {
            update_hr_from_external();
        }
    } else {
        return;
    }

    lastRefreshCharacteristicChanged = now;

    #ifdef Q_OS_IOS
    #ifndef IO_UNDER_QT
    /*
        bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround,
    QZSettings::default_ios_peloton_workaround).toBool(); if (ios_peloton_workaround && cadence && h &&
    firstStateChanged) { h->virtualTreadmill_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualTreadmill_setHeartRate((uint8_t)metrics_override_heartrate());
        }
    */
    #endif
    #endif

    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void ypooelliptical::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    bool iconsole_elliptical = settings.value(QZSettings::iconsole_elliptical, QZSettings::default_iconsole_elliptical).toBool();    
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        QBluetoothUuid _gattCustomService((quint16)0xFFF0);
        QBluetoothUuid _gattFTMSService((quint16)0x1826);
        if (s->serviceUuid() != _gattCustomService && iconsole_elliptical) {
            qDebug() << "skipping service" << s->serviceUuid();
            continue;
        }
        else if(s->serviceUuid() != _gattFTMSService && SCH_590E) {
            qDebug() << "skipping service" << s->serviceUuid();
            continue;            
        }

        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &ypooelliptical::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &ypooelliptical::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &ypooelliptical::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &ypooelliptical::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &ypooelliptical::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &ypooelliptical::descriptorRead);

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

                QBluetoothUuid _gattFTMSWriteCharControlPointId((quint16)0x2AD9);
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattFTMSWriteCharControlPointId) {
                    qDebug() << QStringLiteral("FTMS service and Control Point found");
                    gattFTMSWriteCharControlPointId = c;
                    gattFTMSService = s;
                }                

                QBluetoothUuid _gattWriteCharControlPointId((quint16)0xFFF2);
                if (c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("Custom service and Control Point found");
                    gattWriteCharControlPointId = c;
                    gattCustomService = s;
                }
            }
        }
    }

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {
        QSettings settings;
        if (!this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    emit debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &ypooelliptical::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &ypooelliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    emit debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &ypooelliptical::changeInclinationRequested);
                    connect(virtualBike, &virtualbike::changeInclination, this, &ypooelliptical::changeInclination);
                    /*connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this,
                            &ypooelliptical::ftmsCharacteristicChanged);*/
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
            }
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void ypooelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

/*
void ypooelliptical::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {
    QByteArray b = newValue;
    if (gattWriteCharControlPointId.isValid()) {
        qDebug() << "routing FTMS packet to the bike from virtualBike" << characteristic.uuid() << newValue.toHex(' ');

        // handling reading current resistance
        if (b.at(0) == 0x11) {
            int16_t slope = (((uint8_t)b.at(3)) + (b.at(4) << 8));
            Resistance = (slope / 33) + default_resistance;
        }

        gattCustomService->writeCharacteristic(gattWriteCharControlPointId, b);
    }
}*/

void ypooelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    QSettings settings;
    bool iconsole_elliptical = settings.value(QZSettings::iconsole_elliptical, QZSettings::default_iconsole_elliptical).toBool();    
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    if (gattCustomService != nullptr) {
        if(!iconsole_elliptical)
            initRequest = true;
        emit connectedAndDiscovered();
    } else if(E35) {
        initRequest = true;
        emit connectedAndDiscovered();
    }
}

void ypooelliptical::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void ypooelliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void ypooelliptical::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void ypooelliptical::serviceScanDone(void) {
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
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &ypooelliptical::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void ypooelliptical::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("ypooelliptical::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void ypooelliptical::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("ypooelliptical::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void ypooelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;
        if(device.name().toUpper().startsWith(QStringLiteral("SCH_590E"))) {
            SCH_590E = true;
            qDebug() << "SCH_590E workaround ON!";
        } else if(device.name().toUpper().startsWith(QStringLiteral("E35"))) {
            E35 = true;
            qDebug() << "E35 workaround ON!";
        } else if(device.name().toUpper().startsWith(QStringLiteral("KETTLER "))) {
            KETTLER = true;
            qDebug() << "KETTLER workaround ON!";
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &ypooelliptical::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &ypooelliptical::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &ypooelliptical::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &ypooelliptical::controllerStateChanged);

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

bool ypooelliptical::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t ypooelliptical::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void ypooelliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

double ypooelliptical::minStepInclination() { return 1.0; }
bool ypooelliptical::inclinationSeparatedFromResistance() {
    if(E35) {
        return true;
    } else {
        return false;
    }
}
