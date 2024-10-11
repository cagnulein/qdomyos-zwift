#include "renphobike.h"
#include "CRC16IBM.h""
#include "devices/ftmsbike/ftmsbike.h"
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

renphobike::renphobike(bool noWriteResistance, bool noHeartService) {

    ergModeSupported = true; // IMPORTANT, only for this bike

    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, SIGNAL(timeout()), this, SLOT(update()));
    refresh->start(500);
}

void renphobike::writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                     bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (gattFTMSService == nullptr) {
        qDebug() << QStringLiteral("gattFTMSService not found! skip writing...");
        return;
    }

    if (wait_for_response) {
        connect(gattFTMSService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    } else {
        connect(gattFTMSService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);

    if (!disable_log)
        debug(" >> " + writeBuffer->toHex(' ') + " // " + info);

    loop.exec();
}

void renphobike::writeCharacteristicCustom(uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                     bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (gattCustomService == nullptr) {
        qDebug() << QStringLiteral("gattCustomService not found! skip writing...");
        return;
    }

    if (wait_for_response) {
        connect(gattCustomService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    } else {
        connect(gattCustomService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCustomService->writeCharacteristic(gattWriteCustomCharControlPointId, *writeBuffer);

    if (!disable_log)
        debug(" >> " + writeBuffer->toHex(' ') + " // " + info);

    loop.exec();
}

void renphobike::forcePower(int16_t requestPower) {
    QSettings settings;
    double watt_gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
    double watt_offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
    double r = ((requestPower / watt_gain) - watt_offset);
    uint8_t write[] = {FTMS_SET_TARGET_POWER, 0x00, 0x00};

    write[1] = ((uint16_t)r) & 0xFF;
    write[2] = ((uint16_t)r) >> 8;

    writeCharacteristic(write, sizeof(write), QStringLiteral("forcePower ") + QString::number(r));
}

void renphobike::forceResistance(resistance_t requestResistance) {
    // requestPower = powerFromResistanceRequest(requestResistance);
    /*
    uint8_t write[] = {FTMS_SET_TARGET_RESISTANCE_LEVEL, 0x00};
    QSettings settings;
    bool renpho_bike_double_resistance =
        settings.value(QZSettings::renpho_bike_double_resistance, QZSettings::default_renpho_bike_double_resistance)
            .toBool();

    if (renpho_bike_double_resistance)
        write[1] = ((uint8_t)(requestResistance));
    else
        write[1] = ((uint8_t)(requestResistance * 2));

    writeCharacteristic(write, sizeof(write), QStringLiteral("forceResistance ") + QString::number(requestResistance));
*/
    uint8_t write[] = {0x00, 0x44, 0x11, 0x0f, 0x84, 0xc0};
    write[2] = requestResistance;
    Resistance = requestResistance;
    quint16 crc = CRC16IBM::calculateCRC(QByteArray(reinterpret_cast<const char*>(write), 3));

    // Log the calculated CRC (for debugging purposes)
    qDebug() << "Calculated CRC:" << QString::number(crc, 16);

    // Replace the last two bytes of the write array with the CRC
    write[3] = static_cast<uint8_t>(crc >> 8);  // High byte of CRC
    write[4] = static_cast<uint8_t>(crc & 0xFF); // Low byte of CRC

    writeCharacteristicCustom(write, sizeof(write), QStringLiteral("forceResistance ") + QString::number(requestResistance));
}

void renphobike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        uint8_t init0[] = {0x00, 0x22, 0x19, 0x2f, 0xc0};
        uint8_t init1[] = {0x00, 0x46, 0x00, 0x6b, 0xf6, 0xc0};
        uint8_t init2[] = {0x00, 0x45, 0x00, 0x3e, 0xa5, 0xc0};
        uint8_t init3[] = {0x00, 0x40, 0x00, 0x02, 0xb9, 0x2f, 0xc0};
        uint8_t init4[] = {0x00, 0x40, 0x00, 0x04, 0xd9, 0xe9, 0xc0};
        uint8_t init5[] = {0x00, 0x40, 0x00, 0x03, 0xa9, 0x0e, 0xc0};
        uint8_t init6[] = {0x00, 0x40, 0x00, 0x05, 0xc9, 0xc8, 0xc0};
        uint8_t init7[] = {0x00, 0x46, 0x00, 0x6b, 0xf6, 0xc0};
        uint8_t init8[] = {0x00, 0x45, 0x01, 0x2e, 0x84, 0xc0};
        uint8_t init9[] = {0x00, 0x40, 0x00, 0x02, 0xb9, 0x2f, 0xc0};
        uint8_t init10[] = {0x00, 0x40, 0x00, 0x04, 0xd9, 0xe9, 0xc0};
        writeCharacteristicCustom(init0, sizeof(init0), "init0", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init1, sizeof(init1), "init1", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init2, sizeof(init2), "init2", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init3, sizeof(init3), "init3", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init4, sizeof(init4), "init4", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init5, sizeof(init5), "init5", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init6, sizeof(init6), "init6", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init7, sizeof(init7), "init7", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init8, sizeof(init8), "init8", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init9, sizeof(init9), "init9", false, true);
        QThread::msleep(1000);
        writeCharacteristicCustom(init10, sizeof(init10), "init10", false, true);
        /*
        uint8_t write[] = {FTMS_REQUEST_CONTROL};
        writeCharacteristic(write, sizeof(write), "requestControl", false, true);
        write[0] = {FTMS_START_RESUME};
        writeCharacteristic(write, sizeof(write), "start simulation", false, true);
        uint8_t ftms[] = {0x11, 0x00, 0x00, 0xf3, 0x00, 0x28, 0x33};
        writeCharacteristic(ftms, sizeof(ftms), "fake FTMS", false, true);*/

        initRequest = false;

    } else if (bluetoothDevice.isValid() &&
               m_control->state() == QLowEnergyController::DiscoveredState //&&
                                                                           // gattCommunicationChannelService &&
                                                                           // gattWriteCharacteristic.isValid() &&
                                                                           // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {
        update_metrics(false, watts());

        if (!autoResistanceEnable) {
            uint8_t write[] = {FTMS_STOP_PAUSE, 0x01};
            writeCharacteristic(write, sizeof(write),
                                QStringLiteral("stopping control ") + QString::number(requestPower));
            return;
        } else {
            auto virtualBike = this->VirtualBike();
            if (requestPower != -1) {
                debug("writing power request " + QString::number(requestPower));
                QSettings settings;
                bool power_sensor =
                    !settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                         .toString()
                         .startsWith(QStringLiteral("Disabled"));
                // if zwift is connected, QZ routes the ftms packets directly to the bike.
                // but if zwift is connected and there is also a power sensor attached, we need to change the erg mode
                // dynamically if peloton is connected, the power request is handled by QZ
                if (((virtualBike && !virtualBike->ftmsDeviceConnected()) || power_sensor) && requestPower != 0)
                    forcePower(ergModificator(requestPower));
                requestPower = -1;
                requestResistance = -1;
            }
            // if zwift is connected we have to avoid to send resistance to the bike
            if ((virtualBike && !virtualBike->ftmsDeviceConnected()) || !virtualBike) {
                if (requestResistance != -1) {
                    if (requestResistance > max_resistance)
                        requestResistance = max_resistance;
                    else if (requestResistance == 0)
                        requestResistance = 1;

                    lastRequestResistance = lastRawRequestedResistanceValue;
                    debug("writing resistance " + QString::number(requestResistance));
                    forceResistance(requestResistance);

                    requestResistance = -1;
                } else if (lastRequestResistance != -1) {
                    int8_t r = lastRequestResistance * m_difficult + gears();
                    debug("writing resistance for renpho forever " + QString::number(r));
                    forceResistance(r);
                }
            } else if (requestResistance != -1 || lastRequestResistance != -1) {
                qDebug() << QStringLiteral("ignoring resistance because ftmsDeviceConnected is connected");
            }
        }
        if (requestStart != -1) {
            debug("starting...");

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            lastRequestResistance = -1;
            debug("stopping...");
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void renphobike::serviceDiscovered(const QBluetoothUuid &gatt) { debug("serviceDiscovered " + gatt.toString()); }

void renphobike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool renpho_bike_double_resistance =
        settings.value(QZSettings::renpho_bike_double_resistance, QZSettings::default_renpho_bike_double_resistance)
            .toBool();

    debug(" << " + newValue.toHex(' '));

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

            {
                if (settings.value(QZSettings::schwinn_bike_resistance, QZSettings::default_schwinn_bike_resistance)
                        .toBool())
                    Resistance = pelotonToBikeResistance(m_pelotonResistance.value());
                else
                    Resistance = m_pelotonResistance;
                emit resistanceRead(Resistance.value());
                qDebug() << QStringLiteral("Current Resistance Calculated: ") + QString::number(Resistance.value());
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

    if (characteristic.uuid() != QBluetoothUuid((quint16)0x2AD2))
        return;

    lastPacket = newValue;

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
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool())
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;
        else
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
        index += 2;
        debug("Current Speed: " + QString::number(Speed.value()));
    }

    if (Flags.avgSpeed) {
        double avgSpeed;
        avgSpeed =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            100.0;
        index += 2;
        debug("Current Average Speed: " + QString::number(avgSpeed));
    }

    if (Flags.instantCadence) {
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith("Disabled"))
            Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                (uint16_t)((uint8_t)newValue.at(index)))) /
                      2.0;
        index += 2;
        debug("Current Cadence: " + QString::number(Cadence.value()));
    }

    if (Flags.avgCadence) {
        double avgCadence;
        avgCadence =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            2.0;
        index += 2;
        debug("Current Average Cadence: " + QString::number(avgCadence));
    }

    if (Flags.totDistance) {
        Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                              (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint32_t)((uint8_t)newValue.at(index)))) /
                   1000.0;
        index += 3;
    } else {
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
    }

    debug("Current Distance: " + QString::number(Distance.value()));

    if (Flags.resistanceLvl) {
        Resistance =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index)))) /
            2;
        if (renpho_bike_double_resistance)
            Resistance = Resistance.value() * 2;
        emit resistanceRead(Resistance.value());
        m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());
        index += 2;
        debug("Current Resistance: " + QString::number(Resistance.value()));
    }

    if (Flags.instantPower) {
        wattFromBike =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            m_watt = wattFromBike.value();
        index += 2;
        debug("Current Watt: " + QString::number(m_watt.value()));
        debug("Current Watt from the Bike: " + QString::number(wattFromBike.value()));
    }

    if (Flags.avgPower) {
        double avgPower;
        avgPower =
            ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint16_t)((uint8_t)newValue.at(index))));
        index += 2;
        debug("Current Average Watt: " + QString::number(avgPower));
    }

    if (Flags.expEnergy) {
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

    debug("Current KCal: " + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (Flags.heartRate) {
            Heart = ((double)(((uint8_t)newValue.at(index))));
            index += 1;
            debug("Current Heart: " + QString::number(Heart.value()));
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

    if (heartRateBeltName.startsWith("Disabled")) {
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

    debug("Current CrankRevs: " + QString::number(CrankRevs));
    debug("Last CrankEventTime: " + QString::number(LastCrankEventTime));

    if (m_control->error() != QLowEnergyController::NoError)
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();
}

void renphobike::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    debug("BTLE stateChanged " + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    foreach (QLowEnergyService *s, gattCommunicationChannelService) {
        qDebug() << "stateChanged" << s->serviceUuid() << s->state();
#ifdef Q_OS_WINDOWS
        QBluetoothUuid ftmsService((quint16)0x1826);
        qDebug() << "windows workaround, check only the ftms service" << (s->serviceUuid() == ftmsService);
        if (s->serviceUuid() == ftmsService)
#endif
        {
            if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
                qDebug() << "not all services discovered";
                return;
            }
        }
    }

    qDebug() << "all services discovered!";

    foreach (QLowEnergyService *s, gattCommunicationChannelService) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, SIGNAL(characteristicChanged(QLowEnergyCharacteristic, QByteArray)), this,
                    SLOT(characteristicChanged(QLowEnergyCharacteristic, QByteArray)));
            connect(s, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)), this,
                    SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
            connect(s, SIGNAL(characteristicRead(const QLowEnergyCharacteristic, const QByteArray)), this,
                    SLOT(characteristicRead(const QLowEnergyCharacteristic, const QByteArray)));
            connect(s, SIGNAL(error(QLowEnergyService::ServiceError)), this,
                    SLOT(errorService(QLowEnergyService::ServiceError)));
            connect(s, SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                    SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));
            connect(s, SIGNAL(descriptorRead(const QLowEnergyDescriptor, const QByteArray)), this,
                    SLOT(descriptorRead(const QLowEnergyDescriptor, const QByteArray)));

            qDebug() << s->serviceUuid() << "connected!";

            foreach (QLowEnergyCharacteristic c, s->characteristics()) {
                qDebug() << "char uuid" << c.uuid() << "handle" << c.handle();
                foreach (QLowEnergyDescriptor d, c.descriptors())
                    qDebug() << "descriptor uuid" << d.uuid() << "handle" << d.handle();

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid())
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    else
                        qDebug() << "ClientCharacteristicConfiguration" << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << " is not valid";

                    qDebug() << s->serviceUuid() << c.uuid() << "notification subscribed!";
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                           QLowEnergyCharacteristic::Indicate) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid())
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    else
                        qDebug() << "ClientCharacteristicConfiguration" << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << " is not valid";

                    qDebug() << s->serviceUuid() << c.uuid() << "indication subscribed!";
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << "FTMS service and Control Point found";
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                }

                QBluetoothUuid _gattWriteCustomCharControlPointId(QStringLiteral("00000004-21a4-11e8-8812-000c2920efff"));
                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCustomCharControlPointId) {
                    qDebug() << "Custom service and Custom Control Point found";
                    gattWriteCustomCharControlPointId = c;
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
            debug("creating virtual bike interface...");
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
            // connect(virtualBike,&virtualbike::debug ,this,&renphobike::debug);
            connect(virtualBike, &virtualbike::changeInclination, this, &renphobike::changeInclination);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &renphobike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

uint16_t renphobike::ergModificator(uint16_t powerRequested) {
    QSettings settings;
    bool power_sensor = !settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                             .toString()
                             .startsWith(QStringLiteral("Disabled"));
    double watt_gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
    double watt_offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();

    qDebug() << QStringLiteral("applying ERG mod from") << powerRequested;
    powerRequested = ((powerRequested / watt_gain) - watt_offset);
    qDebug() << QStringLiteral("to") << powerRequested;

    if (power_sensor && this->VirtualBike()) {
        if (QDateTime::currentMSecsSinceEpoch() > (this->VirtualBike()->whenLastFTMSFrameReceived() + 5000)) {
            double f = ((double)powerRequested * (double)powerRequested) / m_watt.average5s();
            lastPowerRequestedFactor = f / powerRequested;
            powerRequested = f;
            qDebug() << QStringLiteral("power sensor detected, reading from the bike") << wattFromBike.value()
                     << QStringLiteral("reading from power pedal") << m_watt.value()
                     << QStringLiteral("reading from power pedal (avg 5s)") << m_watt.average5s()
                     << QStringLiteral("powerRequested") << powerRequested;
        } else {
            return powerRequested * lastPowerRequestedFactor;
        }
    }
    return powerRequested;
}

void renphobike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    if (!autoResistanceEnable) {
        qDebug() << QStringLiteral(
            "routing FTMS packet to the bike from virtualbike discarded because auto resistance is disabled");
        return;
    }

    lastFTMSPacketReceived.clear();
    for (int i = 0; i < newValue.length(); i++)
        lastFTMSPacketReceived.append(newValue.at(i));

    if (gattWriteCharControlPointId.isValid()) {
        qDebug() << QStringLiteral("routing FTMS packet to the bike from virtualbike") << characteristic.uuid()
                 << newValue.toHex(' ') << lastFTMSPacketReceived.toHex(' ');

        // handling watt gain for erg
        QSettings settings;
        bool power_sensor = !settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                                 .toString()
                                 .startsWith(QStringLiteral("Disabled"));
        double watt_gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
        double watt_offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();
        if (lastFTMSPacketReceived.at(0) == FTMS_SET_TARGET_POWER &&
            (watt_gain != 1.0 || watt_offset != 0 || power_sensor)) {
            uint16_t r = (((uint8_t)lastFTMSPacketReceived.at(1)) + (lastFTMSPacketReceived.at(2) << 8));
            r = ergModificator(r);

            lastFTMSPacketReceived.clear();
            lastFTMSPacketReceived.append(FTMS_SET_TARGET_POWER);
            lastFTMSPacketReceived.append(r & 0xFF);
            lastFTMSPacketReceived.append(((r & 0xFF00) >> 8) & 0x00FF);
            qDebug() << QStringLiteral("sending") << lastFTMSPacketReceived.toHex(' ');
        // handling gears
        } else if (lastFTMSPacketReceived.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) {
            qDebug() << "applying gears mod" << gears();
            int16_t slope = (((uint8_t)lastFTMSPacketReceived.at(3)) + (lastFTMSPacketReceived.at(4) << 8));
            if (gears() != 0) {
                slope += (gears() * 50);
                lastFTMSPacketReceived[3] = slope & 0xFF;
                lastFTMSPacketReceived[4] = slope >> 8;
            }
        }

        if (writeBuffer) {
            delete writeBuffer;
        }
        writeBuffer = new QByteArray(lastFTMSPacketReceived);

        gattFTMSService->writeCharacteristic(gattWriteCharControlPointId, *writeBuffer);
    }
}

void renphobike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    debug("descriptorWritten " + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void renphobike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << "descriptorRead " << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void renphobike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    debug("characteristicWritten " + newValue.toHex(' '));
}

void renphobike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << "characteristicRead " << characteristic.uuid() << newValue.toHex(' ');
}

void renphobike::serviceScanDone(void) {
    debug("serviceScanDone");

#ifdef Q_OS_ANDROID
    QLowEnergyConnectionParameters c;
    c.setIntervalRange(24, 40);
    c.setLatency(0);
    c.setSupervisionTimeout(420);
    m_control->requestConnectionUpdate(c);
#endif

    foreach (QBluetoothUuid s, m_control->services()) {
#ifdef Q_OS_WINDOWS
        QBluetoothUuid ftmsService((quint16)0x1826);
        qDebug() << "windows workaround, check only the ftms service" << s << ftmsService << (s == ftmsService);
        if (s == ftmsService)
#endif
        {
            gattCommunicationChannelService.append(m_control->createServiceObject(s));
            connect(gattCommunicationChannelService.last(), SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this,
                    SLOT(stateChanged(QLowEnergyService::ServiceState)));
            gattCommunicationChannelService.last()->discoverDetails();
        }
    }
}

void renphobike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    debug("renphobike::errorService" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void renphobike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    debug("renphobike::error" + QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void renphobike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, SIGNAL(serviceDiscovered(const QBluetoothUuid &)), this,
                SLOT(serviceDiscovered(const QBluetoothUuid &)));
        connect(m_control, SIGNAL(discoveryFinished()), this, SLOT(serviceScanDone()));
        connect(m_control, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(error(QLowEnergyController::Error)));
        connect(m_control, SIGNAL(stateChanged(QLowEnergyController::ControllerState)), this,
                SLOT(controllerStateChanged(QLowEnergyController::ControllerState)));

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    debug("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            debug("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            debug("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

resistance_t renphobike::pelotonToBikeResistance(int pelotonResistance) {
    for (resistance_t i = 1; i < max_resistance - 1; i++) {
        if (bikeResistanceToPeloton(i) <= pelotonResistance && bikeResistanceToPeloton(i + 1) >= pelotonResistance)
            return i;
    }
    return Resistance.value();
}

// todo, probably the best way is to use the SET_TARGET_POWER over FTMS
/*
uint8_t renphobike::resistanceFromPowerRequest(uint16_t power)
{
    qDebug() << "resistanceFromPowerRequest" << Cadence.value();

    for(int i = 1; i<max_resistance-1; i++)
    {
        if(wattsFromResistance(i) <= power && wattsFromResistance(i+1) >= power)
        {
            qDebug() << "resistanceFromPowerRequest" << wattsFromResistance(i) << wattsFromResistance(i+1) << power;
            return i;
        }
    }
    return Resistance.value();
}*/

double renphobike::bikeResistanceToPeloton(double resistance) {
    QSettings settings;
    bool renpho_peloton_conversion_v2 =
        settings.value(QZSettings::renpho_peloton_conversion_v2, QZSettings::default_renpho_peloton_conversion_v2)
            .toBool();
    bool renpho_bike_double_resistance =
        settings.value(QZSettings::renpho_bike_double_resistance, QZSettings::default_renpho_bike_double_resistance)
            .toBool();

    if (renpho_bike_double_resistance)
        resistance = resistance / 2.0;

    if (!renpho_peloton_conversion_v2) {
        // 0,0069x2 + 0,3538x + 24,207
        double p = ((0.0069 * pow(resistance, 2)) + (0.3538 * resistance) + 24.207);
        if (p < 0)
            p = 0;
        return p;
    } else {
        // y = 0,0283x2 + 0,6748x + 24,518
        double p = ((0.0283 * pow(resistance, 2)) + (0.6748 * resistance) + 24.518);
        if (p < 0)
            p = 0;
        return p;
    }
}

bool renphobike::connected() {
    if (!m_control)
        return false;
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t renphobike::watts() {
    if (currentCadence().value() == 0)
        return 0;

    return m_watt.value();
}

void renphobike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << "controllerStateChanged" << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << "trying to connect back again...";
        initDone = false;
        m_control->connectToDevice();
    }
}
