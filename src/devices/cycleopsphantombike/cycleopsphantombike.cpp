#include "cycleopsphantombike.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif

#include <chrono>

using namespace std::chrono_literals;

cycleopsphantombike::cycleopsphantombike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &cycleopsphantombike::update);
    refresh->start(200ms);
}

void cycleopsphantombike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                   bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    
    if(!gattCustomService) {
        qDebug() << "gattCustomService is null!";
        return;
    }
    
    if (wait_for_response) {
        connect(gattCustomService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCustomService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCustomService->writeCharacteristic(gattWriteCharCustomId, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void cycleopsphantombike::setControlMode(ControlMode mode, int16_t parameter1, int16_t parameter2) {
    QByteArray data;
    data.resize(10);
    data[0] = 0x00;
    data[1] = 0x10;
    data[2] = static_cast<char>(mode);
    data[3] = parameter1 & 0xFF;
    data[4] = (parameter1 >> 8) & 0xFF;
    data[5] = parameter2 & 0xFF;
    data[6] = (parameter2 >> 8) & 0xFF;
    data[7] = 0x00;
    data[8] = 0x00;
    data[9] = 0x00;

    writeCharacteristic((uint8_t*)data.data(), data.size(), QStringLiteral("setControlMode"));
}

// CycleOps trainers require ~3 seconds between manual target messages
// -- they also take 4-5 seconds for the brake to actually engage and track towared the target.
void cycleopsphantombike::changePower(int32_t power) {
    RequestedPower = power;

    if (power < 0)
        power = 0;
    setControlMode(ControlMode::ManualPower, power);
}

void cycleopsphantombike::forceInclination(double inclination) {
    QSettings settings;

    // weight = kg * 100, grade = % * 10
    // the OP says that doesn't work, let's use wattage instead
    //setControlMode(ControlMode::ManualSlope, settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 100.0, inclination * 10.0);
    double bikeResistanceOffset = settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset).toInt();
    double bikeResistanceGain = settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f).toDouble();
    double power = 100.0 + (inclination * 10.0 * bikeResistanceGain) + bikeResistanceOffset;
    setControlMode(ControlMode::ManualPower, power);
}

void cycleopsphantombike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        QSettings settings;
        bool tacx_neo2_peloton =
            settings.value(QZSettings::tacx_neo2_peloton, QZSettings::default_tacx_neo2_peloton).toBool();
        if (tacx_neo2_peloton)
            requestInclination = 0;
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

        auto virtualBike = this->VirtualBike();

        if (requestResistance != -1) {
            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike) &&
                    (requestPower == 0 || requestPower == -1)) {
                    requestInclination = requestResistance / 10.0;
                }
                // forceResistance(requestResistance);;
            }
            requestResistance = -1;
        }
        if (requestInclination != -100 || lastGearValue != gears()) {
            emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination));
            forceInclination(requestInclination + gears()); // since this bike doesn't have the concept of resistance,
                                                            // i'm using the gears in the inclination
            requestInclination = -100;
        } else if((virtualBike && virtualBike->ftmsDeviceConnected()) && lastGearValue != gears() && lastRawRequestedInclinationValue != -100) {
            // in order to send the new gear value ASAP
            forceInclination(lastRawRequestedInclinationValue + gears());   // since this bike doesn't have the concept of resistance,
                                                            // i'm using the gears in the inclination
        }

        lastGearValue = gears();

        if (requestPower != -1) {
            changePower(requestPower);
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

void cycleopsphantombike::powerPacketReceived(const QByteArray &b) {
    Q_UNUSED(b)
    /*
    if(gattPowerService)
        writeCharacteristic((uint8_t*)b.constData(), b.length(), "powerPacketReceived bridge", false, false);
    else
        qDebug() << "no power service found" << b;
        */
}

void cycleopsphantombike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void cycleopsphantombike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool tacx_neo2_peloton =
        settings.value(QZSettings::tacx_neo2_peloton, QZSettings::default_tacx_neo2_peloton).toBool();

    qDebug() << QStringLiteral(" << char ") << characteristic.uuid();
    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));
    uint8_t heart = 0;
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

    const QBluetoothUuid CYCLEOPS_SERVICE_UUID = QBluetoothUuid(QString("C0F4013A-A837-4165-BAB9-654EF70747C6"));
    const QBluetoothUuid CONTROL_POINT_UUID = QBluetoothUuid(QString("CA31A533-A858-4DC7-A650-FDEB6DAD4C14"));

    if (characteristic.uuid() == CONTROL_POINT_UUID) {
        // Parse response based on CycleOps protocol
        if (newValue.length() >= 10) {
            uint16_t commandId = (static_cast<uint8_t>(newValue[3]) << 8) | static_cast<uint8_t>(newValue[2]);
            qDebug() << "commandID" << commandId;

            if (commandId == 0x1000) {
                ControlMode mode = static_cast<ControlMode>(newValue[4]);
                int16_t param1 = (static_cast<uint8_t>(newValue[6]) << 8) | static_cast<uint8_t>(newValue[5]);
                int16_t param2 = (static_cast<uint8_t>(newValue[8]) << 8) | static_cast<uint8_t>(newValue[7]);
                ControlStatus status = static_cast<ControlStatus>(newValue[9]);

                qDebug() << "mode" << QByteArray(newValue[4], 1).toHex(' ') << "status" << QByteArray(newValue[9], 1).toHex(' ') << "param1" << param1 << "param2" << param2;

                // Handle status updates
                switch (status) {
                case ControlStatus::SpeedUp:
                    emit debug(QStringLiteral("Need to speed up"));
                    break;
                case ControlStatus::SpeedDown:
                    emit debug(QStringLiteral("Need to slow down"));
                    break;
                default:
                    break;
                }

                // Update metrics based on parameters if needed
                if (mode == ControlMode::ManualPower) {
                }
            }
        }
        return;
    }

    if (characteristic.uuid() == QBluetoothUuid((quint16)0x2A5B)) {
        lastPacket = newValue;

        uint8_t index = 1;

        if (newValue.at(0) == 0x02 && newValue.length() < 4) {
            emit debug(QStringLiteral("Crank revolution data present with wrong bytes ") +
                       QString::number(newValue.length()));
            return;
        } else if (newValue.at(0) == 0x01 && newValue.length() < 6) {
            emit debug(QStringLiteral("Wheel revolution data present with wrong bytes ") +
                       QString::number(newValue.length()));
            return;
        } else if (newValue.at(0) == 0x00) {
            emit debug(QStringLiteral("Cadence sensor notification without datas ") +
                       QString::number(newValue.length()));
            return;
        }

        if (newValue.at(0) == 0x02) {
            CrankRevsRead =
                (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
        } else if (newValue.at(0) == 0x03) {
            index += 6;
            CrankRevsRead =
                (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));
        } else {
            return;
            // CrankRevsRead = (((uint32_t)((uint8_t)newValue.at(index + 3)) << 24) |
            // ((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) | ((uint32_t)((uint8_t)newValue.at(index + 1)) << 8)
            // | (uint32_t)((uint8_t)newValue.at(index)));
        }
        if (newValue.at(0) == 0x01) {
            index += 4;
        } else {
            index += 2;
        }
        uint16_t LastCrankEventTimeRead =
            (((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)));

        int16_t deltaT = LastCrankEventTimeRead - oldLastCrankEventTime;
        if (deltaT < 0) {
            deltaT = LastCrankEventTimeRead + 65535 - oldLastCrankEventTime;
        }

        // Tacx Neo flywheel spins up when freewheeling in a low virtual gear (Issue #2157)
        if(m_watt.value() == 0) {
            Cadence = 0;
        } else if (CrankRevsRead != oldCrankRevs && deltaT) {
            double cadence = (((double)CrankRevsRead - (double)oldCrankRevs) / (double)deltaT) * 1024.0 * 60.0;
            if (cadence >= 0 && cadence < 255) {
                Cadence = cadence;
            }
            lastGoodCadence = now;
        } else if (lastGoodCadence.msecsTo(now) > 2000) {
            Cadence = 0;
        }

        oldLastCrankEventTime = LastCrankEventTimeRead;
        oldCrankRevs = CrankRevsRead;

        Speed = Cadence.value() *
                settings.value(QZSettings::cadence_sensor_speed_ratio, QZSettings::default_cadence_sensor_speed_ratio)
                    .toDouble();

        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged2A5B.msecsTo(now)));

        // Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
        // (uint16_t)((uint8_t)newValue.at(index)))); debug("Current Resistance: " +
        // QString::number(Resistance.value()));

        if (tacx_neo2_peloton) {
            m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());
        } else {
            double ac = 0.01243107769;
            double bc = 1.145964912;
            double cc = -23.50977444;

            double ar = 0.1469553975;
            double br = -5.841344538;
            double cr = 97.62165482;

            m_pelotonResistance =
                (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                           (cr - (m_watt.value() * 132.0 /
                                                  (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                   br) /
                  (2.0 * ar)) *
                 settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
            Resistance = m_pelotonResistance;
        }
        emit resistanceRead(Resistance.value());

        if (watts())
            KCal +=
                ((((0.048 * ((double)watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged2A5B.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        lastRefreshCharacteristicChanged2A5B = now;

        emit debug(QStringLiteral("Current CrankRevsRead: ") + QString::number(CrankRevsRead));
        emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTimeRead));
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    } else if (characteristic.uuid() == QBluetoothUuid::HeartRateMeasurement) {
        if (newValue.length() > 1 && !disable_hr_frommachinery) {
            Heart = newValue[1];
            heart = Heart.value();
        }

        emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    } else if (characteristic.uuid() == QBluetoothUuid::CyclingPowerMeasurement) {
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
                    lastGoodCadence = now;
                } else if (lastGoodCadence.msecsTo(now) > 2000) {
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
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChangedPower.msecsTo(now)));
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
                     (60000.0 / ((double)lastRefreshCharacteristicChangedPower.msecsTo(
                                    now)))); //(( (0.048* Output in watts +1.19) * body weight
                                                                      // in kg * 3.5) / 200 ) / 60
            emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

            lastRefreshCharacteristicChangedPower = now;
        }
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2AD2)) {

        union flags {
            struct {
                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t instantCadence : 1;
                uint16_t avgCadence : 1;
                uint16_t totDistance : 1;
                uint16_t resistanceLvl : 1;
                uint16_t instantPower : 1;
                uint16_t avgPower : 1;
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
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                  (uint16_t)((uint8_t)newValue.at(index)))) /
                        100.0;
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.instantCadence) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          2.0;
            }
            index += 2;
            emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        }

        if (Flags.avgCadence) {
            double avgCadence;
            avgCadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index)))) /
                         2.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Cadence: ") + QString::number(avgCadence));
        }

        if (Flags.totDistance) {

            /*
             * the distance sent from the most trainers is a total distance, so it's useless for QZ
             *
            Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)newValue.at(index)))) /
                       1000.0;*/
            index += 3;
        }

        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged2AD2.msecsTo(now)));

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.resistanceLvl) {
            Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            emit resistanceRead(Resistance.value());
            index += 2;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
            resistance_received = true;
        }
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
            if (!resistance_received) {
                Resistance = m_pelotonResistance;
                emit resistanceRead(Resistance.value());
                emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
            }
        }


        if (Flags.instantPower) {
            // power table from an user
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                           .toString()
                           .startsWith(QStringLiteral("Disabled")))
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        }

        if (Flags.avgPower && newValue.length() > index + 1) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
        }

        if (Flags.expEnergy && newValue.length() > index + 1) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
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
                          ((double)lastRefreshCharacteristicChanged2AD2.msecsTo(
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
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)(((uint8_t)newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
            heart = Flags.heartRate;
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

        lastRefreshCharacteristicChanged2AD2 = now;
    }

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
        debug("Current Heart: " + QString::number(Heart.value()));
    }
#endif
    if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) && (heart == 0 || disable_hr_frommachinery)) {
        update_hr_from_external();
    }

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)currentHeart().value());
    }
#endif
#endif

    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError)
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
}

void cycleopsphantombike::stateChanged(QLowEnergyService::ServiceState state) {
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
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &cycleopsphantombike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &cycleopsphantombike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &cycleopsphantombike::characteristicRead);
            connect(s, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                    SLOT(errorService(QLowEnergyService::ServiceError)));
            connect(s, &QLowEnergyService::descriptorWritten, this, &cycleopsphantombike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &cycleopsphantombike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics = s->characteristics();
            for (const QLowEnergyCharacteristic &c : characteristics) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
                auto descriptors = c.descriptors();
                for (const QLowEnergyDescriptor &d : descriptors) {
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

                const QBluetoothUuid CONTROL_POINT_UUID = QBluetoothUuid(QString("CA31A533-A858-4DC7-A650-FDEB6DAD4C14"));

                if (c.properties() & QLowEnergyCharacteristic::Write &&
                    c.uuid() == QBluetoothUuid::CyclingPowerControlPoint) {
                    qDebug() << QStringLiteral("CyclingPowerControlPoint found");
                    gattWriteCharControlPointId = c;
                    gattPowerService = s;
                } else if (c.uuid() == CONTROL_POINT_UUID) {
                    qDebug() << QStringLiteral("CustomChar found");
                    gattWriteCharCustomId = c;
                    gattCustomService = s;
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
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, 4, 1);
            connect(virtualBike, &virtualbike::changeInclination, this, &cycleopsphantombike::changeInclination);
            // connect(virtualBike, &virtualbike::powerPacketReceived, this, &cycleopsphantombike::powerPacketReceived);
            // connect(virtualBike, &virtualbike::debug, this, &cycleopsphantombike::debug);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void cycleopsphantombike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void cycleopsphantombike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void cycleopsphantombike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void cycleopsphantombike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void cycleopsphantombike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    auto services = m_control->services();
    for (const QBluetoothUuid &s : services) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &cycleopsphantombike::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void cycleopsphantombike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("cycleopsphantombike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void cycleopsphantombike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("cycleopsphantombike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void cycleopsphantombike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &cycleopsphantombike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &cycleopsphantombike::serviceScanDone);
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, &QLowEnergyController::stateChanged, this, &cycleopsphantombike::controllerStateChanged);

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

bool cycleopsphantombike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t cycleopsphantombike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void cycleopsphantombike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

resistance_t cycleopsphantombike::pelotonToBikeResistance(int pelotonResistance) {
    for (resistance_t i = 0; i < max_resistance; i++) {
        if (bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i + 1) > pelotonResistance) {
            return i;
        }
    }
    if (pelotonResistance < bikeResistanceToPeloton(1))
        return 0;
    else
        return max_resistance;
}

double cycleopsphantombike::bikeResistanceToPeloton(double resistance) {
    QSettings settings;
    bool tacx_neo2_peloton =
        settings.value(QZSettings::tacx_neo2_peloton, QZSettings::default_tacx_neo2_peloton).toBool();

    if (tacx_neo2_peloton) {
        return (resistance * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
               settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
    } else {
        return resistance;
    }
}

