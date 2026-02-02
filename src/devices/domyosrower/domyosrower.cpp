#include "domyosrower.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"
#include "virtualdevices/virtualtreadmill.h"
#include "homeform.h"
#include "qzsettings.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>

#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

domyosrower::domyosrower(bool noWriteResistance, bool noHeartService, bool testResistance, int8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);

    this->testResistance = testResistance;
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;

    initDone = false;
    connect(refresh, &QTimer::timeout, this, &domyosrower::update);
    refresh->start(300ms);
}

domyosrower::~domyosrower() { qDebug() << QStringLiteral("~domyosrower()"); }

void domyosrower::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                      bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if(ftmsRower) {
        qDebug() << "ftmsRower! so don't send anything!";
        return;
    }

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
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void domyosrower::updateDisplay(uint16_t elapsed) {

    // if(bike_type == CHANG_YOW)
    {
        uint8_t display2[] = {0xf0, 0xcd, 0x01, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

        display2[3] = ((((uint16_t)(odometer() * 10))) >> 8) & 0xFF;
        display2[4] = (((uint16_t)(odometer() * 10))) & 0xFF;

        for (uint8_t i = 0; i < sizeof(display2) - 1; i++) {

            display2[26] += display2[i]; // the last byte is a sort of a checksum
        }

        writeCharacteristic(display2, 20, QStringLiteral("updateDisplay2"), false, false);
        writeCharacteristic(&display2[20], sizeof(display2) - 20, QStringLiteral("updateDisplay2"), false, true);
    }

    uint8_t display[] = {0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00};

    display[3] = (elapsed / 60) & 0xFF; // high byte for elapsed time (in seconds)
    display[4] = (elapsed % 60 & 0xFF); // low byte for elapsed time (in seconds)

    display[7] = ((uint8_t)((uint16_t)(currentSpeed().value()) >> 8)) & 0xFF;
    display[8] = (uint8_t)(currentSpeed().value()) & 0xFF;

    display[12] = (uint8_t)currentHeart().value();

    // display[13] = ((((uint8_t)calories())) >> 8) & 0xFF;
    // display[14] = (((uint8_t)calories())) & 0xFF;

    display[16] = (uint8_t)currentCadence().value();

    display[19] = ((((uint16_t)calories().value())) >> 8) & 0xFF;
    display[20] = (((uint16_t)calories().value())) & 0xFF;

    for (uint8_t i = 0; i < sizeof(display) - 1; i++) {

        display[26] += display[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(display, 20, QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, false);
    writeCharacteristic(&display[20], sizeof(display) - 20,
                        QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, true);
}

void domyosrower::forceInclination(int8_t requestInclination) {
    uint8_t write[] = {0xf0, 0xe3, 0x00, 0x00};

    write[2] = requestInclination + 1;

    for (uint8_t i = 0; i < sizeof(write) - 1; i++) {

        write[3] += write[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(write, sizeof(write),
                        QStringLiteral("forceInclination ") + QString::number(requestInclination));
}

void domyosrower::forceResistance(resistance_t requestResistance) {
    uint8_t write[] = {0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                       0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x01, 0xff, 0xff, 0xff, 0x00};

    write[10] = requestResistance;

    for (uint8_t i = 0; i < sizeof(write) - 1; i++) {

        write[22] += write[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(write, 20, QStringLiteral("forceResistance ") + QString::number(requestResistance));
    writeCharacteristic(&write[20], sizeof(write) - 20,
                        QStringLiteral("forceResistance ") + QString::number(requestResistance));
}

void domyosrower::update() {

    uint8_t noOpData[] = {0xf0, 0xac, 0x9c};

    // stop tape
    uint8_t initDataF0C800B8[] = {0xf0, 0xc8, 0x00, 0xb8};

    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        // if(bike_type == CHANG_YOW)
        if(!ftmsRower)
            btinit_changyow(false);
        // else
        //    btinit_telink(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               ((gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
                 gattNotifyCharacteristic.isValid() && initDone) || ftmsRower)) {

        update_metrics(true, watts());

        // ******************************************* virtual treadmill init *************************************
        QSettings settings;
        if (!firstVirtual && searchStopped && !this->hasVirtualDevice()) {
            bool virtual_device_rower =
                settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
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
        {
            if (virtual_device_enabled) {
                if (virtual_device_rower) {
                    qDebug() << QStringLiteral("creating virtual rower interface...");
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    // connect(virtualRower,&virtualrower::debug ,this,&echelonrower::debug);
                    this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                } else if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &domyosrower::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &domyosrower::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset,
                                                       bikeResistanceGain);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &domyosrower::changeInclinationRequested);
                    connect(virtualBike, &virtualbike::changeInclination, this, &domyosrower::changeInclination);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
            }
            }
        }
        firstVirtual = 1;

        // ********************************************************************************************************

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {

            sec1Update = 0;
            updateDisplay(elapsed.value());
        } else {
            writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), true, true);
        }

        if (requestResistance != -1) {
            if (requestResistance > 15) {
                requestResistance = 15;
            } else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));

                forceResistance(requestResistance);
            }
            requestResistance = -1;
        } else if (requestInclination != -100) {
            if (requestInclination > 15) {
                requestInclination = 15;
            } else if (requestInclination == 0) {
                requestInclination = 1;
            }

            if (requestInclination != currentInclination().value()) {
                emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));

                forceInclination(requestInclination);
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

            // if(bike_type == CHANG_YOW)
            btinit_changyow(true);
            // else
            //    btinit_telink(true);

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), QStringLiteral("stop tape"));

            requestStop = -1;
        }
    }
}

void domyosrower::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void domyosrower::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void domyosrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    qDebug() << QStringLiteral(" << ") + QString::number(newValue.length()) + QStringLiteral(" ") + newValue.toHex(' ');
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    lastPacket = newValue;
    if(!ftmsRower) {
        if (newValue.length() != 26) {
            return;
        }

        if (newValue.at(22) == 0x06) {
            emit debug(QStringLiteral("inclination up button pressed!"));

            // requestStart = 1;
        } else if (newValue.at(22) == 0x07) {
            emit debug(QStringLiteral("inclination down button pressed!")); // i guess it should be the inclination down

            // requestStop = 1;
        }

        /*if ((uint8_t)newValue.at(1) != 0xbc && newValue.at(2) != 0x04)  // intense run, these are the bytes for the
           inclination and speed status return;*/

        double speed =
            GetSpeedFromPacket(newValue) *
            settings.value(QZSettings::domyos_elliptical_speed_ratio, QZSettings::default_domyos_elliptical_speed_ratio)
                .toDouble();
        double kcal = GetKcalFromPacket(newValue);
        double distance =
            GetDistanceFromPacket(newValue) *
            settings.value(QZSettings::domyos_elliptical_speed_ratio, QZSettings::default_domyos_elliptical_speed_ratio)
                .toDouble();

        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = ((uint8_t)newValue.at(9));
        }
        Resistance = newValue.at(14);
        Inclination = newValue.at(21);
        if (Resistance.value() < 1) {
            emit debug(QStringLiteral("invalid resistance value ") + QString::number(Resistance.value()) +
                       QStringLiteral(" putting to default"));
            Resistance = 1;
        }
        if (Inclination.value() < 0 || Inclination.value() > 15) {
            emit debug(QStringLiteral("invalid inclination value ") + QString::number(Inclination.value()) +
                       QStringLiteral(" putting to default"));
            Inclination.setValue(0);
        }

    #ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
    #endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
                Heart = ((uint8_t)newValue.at(18));
            }
        }

        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));

        emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
        emit debug(QStringLiteral("Current cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Current resistance: ") + QString::number(Resistance.value()));
        emit debug(QStringLiteral("Current inclination: ") + QString::number(Inclination.value()));
        emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
        emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(distance));
        emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
        emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

        if (m_control->error() != QLowEnergyController::NoError) {
            qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
        }

        Speed = speed;
        KCal = kcal;
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
        lastRefreshCharacteristicChanged = now;
    } else {
        union flags {
            struct {

                uint16_t moreData : 1;
                uint16_t avgStroke : 1;
                uint16_t totDistance : 1;
                uint16_t instantPace : 1;
                uint16_t avgPace : 1;
                uint16_t instantPower : 1;
                uint16_t avgPower : 1;
                uint16_t resistanceLvl : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;
        int index = 0;
        double cadence_divider = 2.0;
        if(newValue.length() < 2) {
            qDebug() << "index out of range" << 0;
            return;
        }
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {

            if(index >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }
            Cadence = ((uint8_t)newValue.at(index)) / cadence_divider;

            if(index + 2 >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }
            StrokesCount =
                (((uint16_t)((uint8_t)newValue.at(index + 2)) << 8) | (uint16_t)((uint8_t)newValue.at(index + 1)));

            if (lastStrokesCount != StrokesCount.value()) {
                lastStroke = now;
            }
            lastStrokesCount = StrokesCount.value();

            index += 3;

            /*
             * the concept 2 sends the pace in 2 frames, so this condition will create a bogus speed
            if (!Flags.instantPace) {
                // eredited by echelon rower, probably we need to change this
                Speed = (0.37497622 * ((double)Cadence.value())) / 2.0;
                emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
            }*/
            emit debug(QStringLiteral("Strokes Count: ") + QString::number(StrokesCount.value()));
        }

        if (Flags.avgStroke) {

            double avgStroke;
            if(index >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }            
            avgStroke = ((double)(uint16_t)((uint8_t)newValue.at(index))) / cadence_divider;
            index += 1;
            emit debug(QStringLiteral("Current Average Stroke: ") + QString::number(avgStroke));
        }

        if (Flags.totDistance) {
            if(index + 2 >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }            
            Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)newValue.at(index)))) /
                       1000.0;
            index += 3;
        } else {
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.instantPace) {

            if(index + 1 >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }

            double instantPace;
            instantPace =
                ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Pace: ") + QString::number(instantPace));

            Speed = (60.0 / instantPace) *
                30.0; // translating pace (min/500m) to km/h in order to match the pace function in the rower.cpp
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgPace) {

            if(index + 1>= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }

            double avgPace;
            avgPace =
                ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Average Pace: ") + QString::number(avgPace));
        }

        if (Flags.instantPower) {
            if(index + 1 >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }

            double watt =
                ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            m_watt = watt;
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        }

        if (Flags.avgPower) {
            if(index + 1 >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }

            double avgPower;
            avgPower =
                ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
        }

        if (Flags.resistanceLvl) {
            if(index + 1 >= newValue.length()) {
                qDebug() << "index out of range" << index;
                return;
            }
            Resistance =
                ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
            emit resistanceRead(Resistance.value());
            index += 2;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        }

        if (Flags.expEnergy && index + 1 < newValue.length()) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (watts())
                KCal +=
                    ((((0.048 * ((double)watts()) + 1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
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
            if (Flags.heartRate && !disable_hr_frommachinery) {
                if (index < newValue.length()) {
                    Heart = ((double)(((uint8_t)newValue.at(index))));
                    // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
                    emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
                } else
                    emit debug(QStringLiteral("Error on parsing heart"));
            }
        }

        if (Flags.metabolic) {

            // todo
        }

        if (Flags.elapsedTime) {

            // todo
        }

        if (Flags.remainingTime) {

            // todo
        }

        if (Cadence.value() > 0) {

            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        lastRefreshCharacteristicChanged = now;

        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
        }

    #ifdef Q_OS_IOS
    #ifndef IO_UNDER_QT
        bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
        if (ios_peloton_workaround && cadence && h && firstVirtual) {

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
}

double domyosrower::GetSpeedFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(6) << 8) | ((uint8_t)packet.at(7));
    if (convertedData > 65000 || convertedData == 0 || currentCadence().value() == 0)
        return 0;
    return (60.0 / (double)(convertedData)) * 30.0;
}

double domyosrower::GetKcalFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(10) << 8) | ((uint8_t)packet.at(11));
    return (double)convertedData;
}

double domyosrower::GetDistanceFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(12) << 8) | ((uint8_t)packet.at(13));
    double data = ((double)convertedData) / 10.0f;
    return data;
}

void domyosrower::btinit_changyow(bool startTape) {

    // set speed and incline to 0
    uint8_t initData1[] = {0xf0, 0xc8, 0x01, 0xb9};
    uint8_t initData2[] = {0xf0, 0xc9, 0xb9};

    // main startup sequence
    uint8_t initDataStart[] = {0xf0, 0xa3, 0x93};
    uint8_t initDataStart2[] = {0xf0, 0xa4, 0x94};
    uint8_t initDataStart3[] = {0xf0, 0xa5, 0x95};
    uint8_t initDataStart4[] = {0xf0, 0xab, 0x9b};
    uint8_t initDataStart5[] = {0xf0, 0xc4, 0x03, 0xb7};
    uint8_t initDataStart6[] = {0xf0, 0xad, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff};

    uint8_t initDataStart7[] = {0xff, 0xff, 0x8b}; // power on bt icon
    uint8_t initDataStart8[] = {0xf0, 0xcb, 0x02, 0x00, 0x08, 0xff, 0xff, 0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00};

    uint8_t initDataStart9[] = {0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xb6}; // power on bt word
    uint8_t initDataStart10[] = {0xf0, 0xad, 0xff, 0xff, 0x00, 0x05, 0xff, 0xff, 0xff, 0xff,
                                 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0x01, 0xff};

    uint8_t initDataStart11[] = {0xff, 0xff, 0x94}; // start tape
    uint8_t initDataStart12[] = {0xf0, 0xcb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x14, 0x01, 0xff, 0xff};

    uint8_t initDataStart13[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbd};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart, sizeof(initDataStart), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart2, sizeof(initDataStart2), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart3, sizeof(initDataStart3), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart4, sizeof(initDataStart4), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart5, sizeof(initDataStart5), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart6, sizeof(initDataStart6), QStringLiteral("init"), false, false);
    writeCharacteristic(initDataStart7, sizeof(initDataStart7), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart8, sizeof(initDataStart8), QStringLiteral("init"), false, false);
    writeCharacteristic(initDataStart9, sizeof(initDataStart9), QStringLiteral("init"), false, true);
    writeCharacteristic(initDataStart10, sizeof(initDataStart10), QStringLiteral("init"), false, false);
    if (startTape) {
        writeCharacteristic(initDataStart11, sizeof(initDataStart11), QStringLiteral("init"), false, true);
        writeCharacteristic(initDataStart12, sizeof(initDataStart12), QStringLiteral("init"), false, false);
        writeCharacteristic(initDataStart13, sizeof(initDataStart13), QStringLiteral("init"), false, true);
    }

    initDone = true;
}

void domyosrower::btinit_telink(bool startTape) {

    Q_UNUSED(startTape)

    // set speed and incline to 0
    uint8_t initData1[] = {0xf0, 0xc8, 0x01, 0xb9};
    uint8_t initData2[] = {0xf0, 0xc9, 0xb9};
    uint8_t noOpData[] = {0xf0, 0xac, 0x9c};

    // main startup sequence
    uint8_t initDataStart[] = {0xf0, 0xcc, 0xff, 0xff, 0xff, 0xff, 0x01, 0xff, 0xb8};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"));
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"));
    writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"));
    writeCharacteristic(initDataStart, sizeof(initDataStart), QStringLiteral("init"));
    updateDisplay(0);

    initDone = true;
}

void domyosrower::stateChanged(QLowEnergyService::ServiceState state) {
    if(!ftmsRower) {
        QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
        QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));

        QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
        emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

        if (state == QLowEnergyService::ServiceDiscovered) {

            // qDebug() << gattCommunicationChannelService->characteristics();

            gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
            gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
            Q_ASSERT(gattWriteCharacteristic.isValid());
            Q_ASSERT(gattNotifyCharacteristic.isValid());

            // establish hook into notifications
            connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                    &domyosrower::characteristicChanged);
            connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                    &domyosrower::characteristicWritten);
            connect(gattCommunicationChannelService,
                    static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                    this, &domyosrower::errorService);
            connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                    &domyosrower::descriptorWritten);

            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            gattCommunicationChannelService->writeDescriptor(
                gattNotifyCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        }
    } else {
        QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
        emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

        for (QLowEnergyService *s : qAsConst(gattCommunicationChannelServiceArray)) {
            qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
            if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
                qDebug() << QStringLiteral("not all services discovered");

                return;
            }
        }

        qDebug() << QStringLiteral("all services discovered!");

        for (QLowEnergyService *s : qAsConst(gattCommunicationChannelServiceArray)) {
            if (s->state() == QLowEnergyService::ServiceDiscovered) {

                // establish hook into notifications
                connect(s, &QLowEnergyService::characteristicChanged, this, &domyosrower::characteristicChanged);
                connect(s, &QLowEnergyService::characteristicWritten, this, &domyosrower::characteristicWritten);
                connect(s, &QLowEnergyService::characteristicRead, this, &domyosrower::characteristicRead);
                connect(
                    s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                    this, &domyosrower::errorService);
                connect(s, &QLowEnergyService::descriptorWritten, this, &domyosrower::descriptorWritten);
                connect(s, &QLowEnergyService::descriptorRead, this, &domyosrower::descriptorRead);

                qDebug() << s->serviceUuid() << QStringLiteral("connected!");

                auto characteristics_list = s->characteristics();
                for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                    qDebug() << "char uuid" << c.uuid() << QStringLiteral("handle") << c.handle();
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

                    QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
                    if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                        qDebug() << QStringLiteral("FTMS service and Control Point found");

                        gattWriteCharControlPointId = c;
                        gattFTMSService = s;
                    }
                }
            }
        }
    }
}

void domyosrower::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void domyosrower::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void domyosrower::searchingStop() { searchStopped = true; }

void domyosrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void domyosrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void domyosrower::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if(gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &domyosrower::stateChanged);
        gattCommunicationChannelService->discoverDetails();
    } else {
        // Main service not found, check if FTMS service is available
        QBluetoothUuid ftmsServiceId((quint16)0x1826);
        QLowEnergyService *ftmsService = m_control->createServiceObject(ftmsServiceId);
        if(ftmsService) {
            QSettings settings;
            settings.setValue(QZSettings::ftms_rower, bluetoothDevice.name());
            qDebug() << "forcing FTMS rower since it has FTMS service but not the main domyos service";
            if(homeform::singleton())
                homeform::singleton()->setToastRequested("FTMS rower found, restart the app to apply the change");
            delete ftmsService;
        }

        ftmsRower = true;
        auto services_list = m_control->services();
        for (const QBluetoothUuid &s : qAsConst(services_list)) {
            gattCommunicationChannelServiceArray.append(m_control->createServiceObject(s));
            connect(gattCommunicationChannelServiceArray.constLast(), &QLowEnergyService::stateChanged, this,
                    &domyosrower::stateChanged);
            gattCommunicationChannelServiceArray.constLast()->discoverDetails();
        }
    }
}

void domyosrower::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("domyosrower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void domyosrower::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("domyosrower::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void domyosrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        if (device.address().toString().startsWith(QStringLiteral("57"))) {
            emit debug(QStringLiteral("domyos telink bike found"));

            bike_type = TELINK;
        } else {
            emit debug(QStringLiteral("domyos changyow bike found"));

            bike_type = CHANG_YOW;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &domyosrower::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &domyosrower::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &domyosrower::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &domyosrower::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
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

bool domyosrower::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t domyosrower::watts() {

    QSettings settings;

    // calc Watts ref. https://alancouzens.com/blog/Run_Power.html

    uint16_t watts = 0;
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    if (currentSpeed().value() > 0) {

        double pace = 60 / currentSpeed().value();
        double VO2R = 210.0 / pace;
        double VO2A = (VO2R * weight) / 1000.0;
        double hwatts = 75 * VO2A;
        double vwatts = ((9.8 * weight) * (currentInclination().value() / 100.0));
        watts = hwatts + vwatts;
    }
    return watts;
}

void domyosrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}
