#include "wahookickrsnapbike.h"
#include "homeform.h"
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

wahookickrsnapbike::wahookickrsnapbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                       double bikeResistanceGain) {
    ergModeSupported = true; // IMPORTANT, only for this bike

    m_watt.setType(metric::METRIC_WATT);
    m_rawWatt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &wahookickrsnapbike::update);
    QSettings settings;
    refresh->start(settings.value(QZSettings::poll_device_time, QZSettings::default_poll_device_time).toInt());
    wheelCircumference::GearTable g;
    g.printTable();
}

void wahookickrsnapbike::restoreDefaultWheelDiameter() {
    // Default wheel circumference is 2070 (700 x 18C)
    QByteArray a = setWheelCircumference(2070);
    uint8_t b[20];
    memcpy(b, a.constData(), a.length());
    writeCharacteristic(b, a.length(), "setWheelCircumference (restore default)", false, true);
    emit debug("Restored default wheel diameter (2070mm) to trainer");
}

bool wahookickrsnapbike::writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                             bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (gattPowerChannelService == nullptr) {
        qDebug() << QStringLiteral("gattPowerChannelService not found, write skipping...");
        return false;
    }

    if (wait_for_response) {
        connect(gattPowerChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(1000, &loop, SLOT(quit()));
    } else {
        connect(gattPowerChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(1000, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattPowerChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log)
        debug(" >> " + writeBuffer->toHex(' ') + " // " + info);

    loop.exec();

    return true;
}

QByteArray wahookickrsnapbike::unlockCommand() {
    QByteArray r;
    r.append(_unlock);
    r.append(0xee);
    r.append(0xfc);
    return r;
}

QByteArray wahookickrsnapbike::setResistanceMode(double resistance) {
    QByteArray r;
    uint16_t norm = (uint16_t)((1 - resistance) * 16383);
    r.append(_setResistanceMode);
    r.append((uint8_t)(norm & 0xFF));
    r.append((uint8_t)(norm >> 8 & 0xFF));
    return r;
}

QByteArray wahookickrsnapbike::setStandardMode(uint8_t level) {
    QByteArray r;
    r.append(_setStandardMode);
    r.append(level);
    return r;
}

QByteArray wahookickrsnapbike::setErgMode(uint16_t watts) {
    QByteArray r;
    r.append(_setErgMode);
    r.append((uint8_t)(watts & 0xFF));
    r.append((uint8_t)(watts >> 8 & 0xFF));
    lastCommandErgMode = true;
    return r;
    // response: 0x01 0x42 0x01 0x00 watts1 watts2
}

QByteArray wahookickrsnapbike::setSimMode(double weight, double rollingResistanceCoefficient,
                                          double windResistanceCoefficient) {
    // Weight units are Kg
    // TODO: Throw Error if weight, rrc or wrc are not within "sane" values
    QByteArray r;
    uint16_t weightN = (uint16_t)(qMax(0.0, qMin(655.35, weight)) * 100);
    uint16_t rrcN = (uint16_t)(qMax(0.0, qMin(65.535, rollingResistanceCoefficient)) * 1000);
    uint16_t wrcN = (uint16_t)(qMax(0.0, qMin(65.535, windResistanceCoefficient)) * 1000);

    r.append(_setSimMode);
    r.append((uint8_t)(weightN & 0xFF));
    r.append((uint8_t)(weightN >> 8 & 0xFF));
    r.append((uint8_t)(rrcN & 0xFF));
    r.append((uint8_t)(rrcN >> 8 & 0xFF));
    r.append((uint8_t)(wrcN & 0xFF));
    r.append((uint8_t)(wrcN >> 8 & 0xFF));
    return r;
}

QByteArray wahookickrsnapbike::setSimCRR(double rollingResistanceCoefficient) {
    // TODO: Throw Error if rrc is not within "sane" value range
    QByteArray r;
    uint16_t rrcN = (uint16_t)(qMax(0.0, qMin(65.535, rollingResistanceCoefficient)) * 1000);
    r.append(_setSimCRR);
    r.append((uint8_t)(rrcN & 0xFF));
    r.append((uint8_t)(rrcN >> 8 & 0xFF));
    return r;
}

QByteArray wahookickrsnapbike::setSimWindResistance(double windResistanceCoefficient) {
    // TODO: Throw Error if wrc is not within "sane" value range
    QByteArray r;
    uint16_t wrcN = (uint16_t)(qMax(0.0, qMin(65.535, windResistanceCoefficient)) * 1000);
    r.append(_setSimWindResistance);
    r.append((uint8_t)(wrcN & 0xFF));
    r.append((uint8_t)(wrcN >> 8 & 0xFF));
    return r;
}

QByteArray wahookickrsnapbike::setSimGrade(double grade) {
    // TODO: Throw Error if grade is not between -1 and 1
    grade = grade / 100;
    QByteArray r;
    uint16_t norm = (uint16_t)((qMin(1.0, qMax(-1.0, grade)) + 1.0) * 65535 / 2.0);
    r.append(_setSimGrade);
    r.append((uint8_t)(norm & 0xFF));
    r.append((uint8_t)(norm >> 8 & 0xFF));
    return r;
}

QByteArray wahookickrsnapbike::setSimWindSpeed(double metersPerSecond) {
    QByteArray r;
    uint16_t norm = (uint16_t)((qMax(-32.767, qMin(32.767, metersPerSecond)) + 32.767) * 1000);
    r.append(_setSimWindSpeed);
    r.append((uint8_t)(norm & 0xFF));
    r.append((uint8_t)(norm >> 8 & 0xFF));
    return r;
}
QByteArray wahookickrsnapbike::setWheelCircumference(double millimeters) {
    QByteArray r;
    uint16_t norm = (uint16_t)(millimeters * 10);
    r.append(_setWheelCircumference);
    r.append((uint8_t)(norm & 0xFF));
    r.append((uint8_t)(norm >> 8 & 0xFF));
    return r;
}

void wahookickrsnapbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        lastCommandErgMode = false;
        QSettings settings;
        QByteArray a = unlockCommand();
        uint8_t b[20];
        memcpy(b, a.constData(), a.length());
        if(!writeCharacteristic(b, a.length(), "init", false, true)) {
            return;
        }
        QThread::msleep(700);

        QByteArray c = setSimMode(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat(), 0.004,
                                  0.4); // wind and rolling should arrive from FTMS
        memcpy(b, c.constData(), c.length());
        if(!writeCharacteristic(b, c.length(), "setSimMode", false, true)) {
            return;
        }
        QThread::msleep(700);

        QByteArray d = setWheelCircumference(wheelCircumference::gearsToWheelDiameter(gears()));
        uint8_t e[20];
        setGears(settings.value(QZSettings::gears_current_value, QZSettings::default_gears_current_value).toDouble());
        memcpy(e, d.constData(), d.length());
        writeCharacteristic(e, d.length(), "setWheelCircumference", false, true);

        // required to the SS2K only one time
        Resistance = 0;
        emit resistanceRead(Resistance.value());
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

        if (requestPower != -1) {
            debug("writing power request " + QString::number(requestPower));
            QSettings settings;
            lastForcedResistance = -1;
            bool power_sensor = !settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                                     .toString()
                                     .startsWith(QStringLiteral("Disabled"));
            
            QByteArray a = setErgMode(requestPower);
            uint8_t b[20];
            memcpy(b, a.constData(), a.length());
            writeCharacteristic(b, a.length(), "setErgMode", false, false);
            requestPower = -1;
            requestResistance = -1;
        }

        if (KICKR_BIKE) {
            if(requestInclination != -100) {
                debug("writing inclination request " + QString::number(requestInclination));
                inclinationChanged(requestInclination, requestInclination);
                Inclination = requestInclination; // the bike is not sending back the inclination?
                requestInclination = -100;
            }
        } else if (requestResistance != -1 && KICKR_BIKE == false) {
            if (requestResistance > 100) {
                requestResistance = 100;
            } else if (requestResistance == 0) {
                requestResistance = 1;
            }

            auto virtualBike = this->VirtualBike();
            if (requestResistance != currentResistance().value() &&
                ((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike)) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                lastForcedResistance = requestResistance;
                QByteArray a = setResistanceMode(((double)requestResistance) / 100.0);
                uint8_t b[20];
                memcpy(b, a.constData(), a.length());
                writeCharacteristic(b, a.length(), "setResistance", false, false);
            } else if (requestResistance != currentResistance().value() &&
               ((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike)) {
               emit debug(QStringLiteral("writing resistance ") + QString::number(lastForcedResistance));
               QByteArray a = setResistanceMode(((double)lastForcedResistance) / 100.0);
               uint8_t b[20];
               memcpy(b, a.constData(), a.length());
               writeCharacteristic(b, a.length(), "setResistance", false, false);
            }
            requestResistance = -1;
        }

        if (lastGearValue != gears()) {
            if(KICKR_SNAP) {
               inclinationChanged(lastGrade, lastGrade);
            } else {
                QByteArray a = setWheelCircumference(wheelCircumference::gearsToWheelDiameter(gears()));
                uint8_t b[20];
                memcpy(b, a.constData(), a.length());
                writeCharacteristic(b, a.length(), "setWheelCircumference", false, false);
                lastGrade = 999; // to force a change
            }
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

void wahookickrsnapbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

resistance_t wahookickrsnapbike::pelotonToBikeResistance(int pelotonResistance) {
    QSettings settings;
    bool schwinn_bike_resistance_v2 =
        settings.value(QZSettings::schwinn_bike_resistance_v2, QZSettings::default_schwinn_bike_resistance_v2).toBool();
    if (!schwinn_bike_resistance_v2) {
        if (pelotonResistance > 54)
            return (pelotonResistance * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
        if (pelotonResistance < 26)
            return ((pelotonResistance / 5) * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();

        // y = 0,04x2 - 1,32x + 11,8
        return (((0.04 * pow(pelotonResistance, 2)) - (1.32 * pelotonResistance) + 11.8) * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
    } else {
        if (pelotonResistance > 20)
            return ((((double)pelotonResistance - 20.0) * 1.25) * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
        else
            return (1  * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
    }
}

uint16_t wahookickrsnapbike::wattsFromResistance(double resistance) {
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

void wahookickrsnapbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << QStringLiteral(" << ") << newValue.toHex(' ') << characteristic.uuid();

    if (characteristic.uuid() == QBluetoothUuid::CyclingPowerMeasurement) {
        lastPacket = newValue;

        uint16_t flags = (((uint16_t)((uint8_t)newValue.at(1)) << 8) | (uint16_t)((uint8_t)newValue.at(0)));
        bool cadence_present = false;
        bool wheel_revs = false;
        bool crank_rev_present = false;
        uint16_t time_division = 1024;
        uint8_t index = 4;

        if (newValue.length() > 3) {
            m_rawWatt = (((uint16_t)((uint8_t)newValue.at(3)) << 8) | (uint16_t)((uint8_t)newValue.at(2)));
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                m_watt = m_rawWatt.value();
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

            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name).toString().startsWith(QStringLiteral("Disabled")) && 
                    settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name).toString().startsWith(QStringLiteral("Disabled"))) {
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

            if (ResistanceFromFTMSAccessory.value() == 0) {
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

                if (isnan(res))
                    m_pelotonResistance = 0;
                else
                    m_pelotonResistance = res;

                if (lastForcedResistance == -1) {
                    if (settings.value(QZSettings::schwinn_bike_resistance, QZSettings::default_schwinn_bike_resistance)
                            .toBool())
                        Resistance = pelotonToBikeResistance(m_pelotonResistance.value());
                    else
                        Resistance = m_pelotonResistance;
                } else {
                    // since I can't read the actual value of the resistance of the trainer, I'm using the last one sent
                    // as the actual value in resistance mode
                    Resistance = lastForcedResistance;
                }
                emit resistanceRead(Resistance.value());
            } else {
                Resistance = ResistanceFromFTMSAccessory.value();
            }

            qDebug() << QStringLiteral("Current Resistance: ") << Resistance.value();
            qDebug() << QStringLiteral("Current Peloton Resistance: ") << m_pelotonResistance.value();

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

        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    }

    {
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

    {
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

void wahookickrsnapbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharCustomService(QStringLiteral("A026E005-0A7D-4AB3-97FA-F1500F9FEB8B"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    notificationSubscribed = 0;

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &wahookickrsnapbike::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &wahookickrsnapbike::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &wahookickrsnapbike::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &wahookickrsnapbike::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &wahookickrsnapbike::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &wahookickrsnapbike::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharCustomService) {
                    qDebug() << QStringLiteral("Custom service and Control Point found");
                    gattWriteCharacteristic = c;
                    gattPowerChannelService = s;
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    notificationSubscribed++;
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
                    notificationSubscribed++;
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
            // connect(virtualBike,&virtualbike::debug ,this,&wahookickrsnapbike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &wahookickrsnapbike::inclinationChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void wahookickrsnapbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") << descriptor.name() << newValue.toHex(' ')
             << notificationSubscribed;

    if (notificationSubscribed)
        notificationSubscribed--;

    if (!notificationSubscribed) {
        initRequest = true;
        emit connectedAndDiscovered();
    }
}

void wahookickrsnapbike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void wahookickrsnapbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                               const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void wahookickrsnapbike::characteristicRead(const QLowEnergyCharacteristic &characteristic,
                                            const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void wahookickrsnapbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    auto services_list = m_control->services();
    bool zwift_found = false;
    bool wahoo_found = false;
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &wahookickrsnapbike::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
        if(s == QBluetoothUuid(QStringLiteral("00000001-19ca-4651-86e5-fa29dcdd09d1"))) {
            zwift_found = true;
        } else if(s == QBluetoothUuid(QStringLiteral("a026ee01-0a7d-4ab3-97fa-f1500f9feb8b"))) {
            wahoo_found = true;
        }
    }

    qDebug() << "zwift service found " << zwift_found << "wahoo service found" << wahoo_found;

    if(zwift_found && !wahoo_found) {
        QSettings settings;
        settings.setValue(QZSettings::ftms_bike, bluetoothDevice.name());
        settings.sync();
        if(homeform::singleton())
            homeform::singleton()->setToastRequested("Zwift Hub device found, please restart the app to enjoy virtual gearing!");
        return;
    }
}

void wahookickrsnapbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("wahookickrsnapbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void wahookickrsnapbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("wahookickrsnapbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void wahookickrsnapbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        if (device.name().toUpper().startsWith("WAHOO KICKR")) {
            WAHOO_KICKR = true;
            qDebug() << "WAHOO KICKR workaround activated";
        } else if(device.name().toUpper().startsWith("KICKR BIKE")) {
            KICKR_BIKE = true;
            qDebug() << "KICKR BIKE workaround activated";
        } else if(device.name().toUpper().startsWith("KICKR SNAP")) {
            KICKR_SNAP = true;
            qDebug() << "KICKR SNAP workaround activated";
        }


        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &wahookickrsnapbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &wahookickrsnapbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &wahookickrsnapbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &wahookickrsnapbike::controllerStateChanged);

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

bool wahookickrsnapbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t wahookickrsnapbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void wahookickrsnapbike::resistanceFromFTMSAccessory(resistance_t res) {
    ResistanceFromFTMSAccessory = res;
    qDebug() << QStringLiteral("resistanceFromFTMSAccessory") << res;
}

void wahookickrsnapbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

void wahookickrsnapbike::inclinationChanged(double grade, double percentage) {
    Q_UNUSED(percentage);
    if(lastCommandErgMode) {
        lastGrade = grade + 1; // to force a refresh
        initRequest = true;
        qDebug() << "avoid sending this command, since I have first to restore the setSimGrade";
        return;
    }
    if(lastGrade == grade) {
        qDebug() << "grade is already set to " << grade << "skipping";
        return;
    }
    lastGrade = grade;
    Inclination = grade;
    emit debug(QStringLiteral("writing inclination ") + QString::number(grade));
    QSettings settings;
    double g = grade;
    if(KICKR_SNAP) {
        g += gears() * 0.5;
        qDebug() << "adding gear offset so " << g;
    }
    QByteArray a = setSimGrade(g);
    uint8_t b[20];
    memcpy(b, a.constData(), a.length());
    writeCharacteristic(b, a.length(), "setSimGrade", false, false);
    lastCommandErgMode = false;
}

bool wahookickrsnapbike::inclinationAvailableByHardware() {
    return KICKR_BIKE;
}

double wahookickrsnapbike::maxGears() {
    wheelCircumference::GearTable g;
    return g.maxGears;
}

double wahookickrsnapbike::minGears() {
    return 1;
}
