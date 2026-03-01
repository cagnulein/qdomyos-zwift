#include "domyosbike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "homeform.h"
#include "qzsettings.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

domyosbike::domyosbike(bool noWriteResistance, bool noHeartService, bool testResistance, int8_t bikeResistanceOffset,
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
    connect(refresh, &QTimer::timeout, this, &domyosbike::update);
    refresh->start(300ms);
}

domyosbike::~domyosbike() { qDebug() << QStringLiteral("~domyosbike()"); }

void domyosbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                     bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &domyosbike::packetReceived, &loop, &QEventLoop::quit);
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

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') +
                        QStringLiteral(" // ") + info;
    }

    loop.exec();

    if (timeout.isActive() == false) {
        qDebug() << QStringLiteral(" exit for timeout");
    }
}

void domyosbike::updateDisplay(uint16_t elapsed) {
    uint16_t multiplier = 1;

    QSettings settings;
    bool distance =
        settings
            .value(QZSettings::domyos_treadmill_distance_display, QZSettings::default_domyos_treadmill_distance_display)
            .toBool();
    bool domyos_bike_display_calories =
        settings.value(QZSettings::domyos_bike_display_calories, QZSettings::default_domyos_bike_display_calories)
            .toBool();

    if (domyos_bike_display_calories) {
        multiplier = 10;
    }

    // if(bike_type == CHANG_YOW)
    if (distance) {
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

    if (currentSpeed().value() < 10.0) {

        display[7] = ((uint16_t)(currentSpeed().value() * multiplier) >> 8) & 0xFF;
        display[8] = (uint16_t)(currentSpeed().value() * multiplier) & 0xFF;
        display[9] = 0x02; // decimal position
    } else if (currentSpeed().value() < 100.0) {

        display[7] = ((uint16_t)(currentSpeed().value() * multiplier) >> 8) & 0xFF;
        display[8] = (uint16_t)(currentSpeed().value() * multiplier) & 0xFF;
        display[9] = 0x01; // decimal position
    } else {

        display[7] = ((uint16_t)(currentSpeed().value() * multiplier) >> 8) & 0xFF;
        display[8] = (uint16_t)(currentSpeed().value() * multiplier) & 0xFF;
        display[9] = 0x00; // decimal position
    }

    display[12] = (uint8_t)currentHeart().value();

    // display[13] = ((((uint8_t)calories())) >> 8) & 0xFF;
    // display[14] = (((uint8_t)calories())) & 0xFF;

    if (domyos_bike_display_calories) {
        display[15] = ((((uint16_t)currentCadence().value()) * multiplier) >> 8) & 0xFF;
        display[16] = (((uint16_t)currentCadence().value()) * multiplier) & 0xFF;
    } else {
        display[16] = ((uint8_t)(currentCadence().value() * multiplier));
    }

    display[19] = ((((uint16_t)calories().value()) * multiplier) >> 8) & 0xFF;
    display[20] = (((uint16_t)calories().value()) * multiplier) & 0xFF;

    for (uint8_t i = 0; i < sizeof(display) - 1; i++) {
        display[26] += display[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(display, 20, QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, false);
    writeCharacteristic(&display[20], sizeof(display) - 20,
                        QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, true);
}

void domyosbike::forceResistance(resistance_t requestResistance) {
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

void domyosbike::update() {

    if (!m_control)
        return;

    uint8_t noOpData[] = {0xf0, 0xac, 0x9c};

    // stop tape
    // uint8_t initDataF0C800B8[] = {0xf0, 0xc8, 0x00, 0xb8}; //unused code

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        // if(bike_type == CHANG_YOW)
        btinit_changyow(false);
        // else
        //    btinit_telink(false);
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotifyCharacteristic.isValid() && initDone) {
        update_metrics(true, watts());

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
                // connect(virtualBike,&virtualbike::debug ,this,&schwinnic4bike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &domyosbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {
            sec1Update = 0;
            if (incompletePackets == false) {
                updateDisplay(elapsed.value());
            }
        } else {
            if (incompletePackets == false) {
                writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), true, true);
            }
        }

        if (incompletePackets == false) {
            if (testResistance) {
                if ((((int)elapsed.value()) % 5) == 0) {
                    uint8_t new_res = currentResistance().value() + 1;
                    if (new_res > 15) {
                        new_res = 1;
                    }
                    forceResistance(new_res);
                }
            }

            if (requestResistance != -1) {
                if (requestResistance > max_resistance) {
                    requestResistance = max_resistance;
                } else if (requestResistance < 1) {
                    requestResistance = 1;
                }

                if (requestResistance != currentResistance().value()) {
                    qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                    forceResistance(requestResistance);
                }
                requestResistance = -1;
            }
            if (requestStart != -1) {
                qDebug() << QStringLiteral("starting...");

                // if(bike_type == CHANG_YOW)
                btinit_changyow(true);
                // else
                //    btinit_telink(true);

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
}

void domyosbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void domyosbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    QByteArray value = newValue;

    qDebug() << QStringLiteral(" << ") + QString::number(value.length()) + QStringLiteral(" ") + value.toHex(' ');

    // for the init packets, the length is always less than 20
    // for the display and status packets, the length is always grater then 20 and there are 2 cases:
    // - intense run: it always send more than 20 bytes in one packets, so the length will be always != 20
    // - t900: it splits packets with length grater than 20 in two distinct packets, so the first one it has length of
    // 20,
    //         and the second one with the remained byte
    // so this simply condition will match all the cases, excluding the 20byte packet of the T900.
    if (newValue.length() != 20) {
        qDebug() << QStringLiteral("packetReceived!");
        initPacketRecv = true;
        emit packetReceived();
    }

    QByteArray startBytes;
    startBytes.append(0xf0);
    startBytes.append(0xbc);

    QByteArray startBytes2;
    startBytes2.append(0xf0);
    startBytes2.append(0xdb);

    QByteArray startBytes3;
    startBytes3.append(0xf0);
    startBytes3.append(0xdd);

    // on some treadmills, the 26bytes has split in 2 packets
    if ((lastPacket.length() == 20 && lastPacket.startsWith(startBytes) && value.length() == 6) ||
        (lastPacket.length() == 20 && lastPacket.startsWith(startBytes2) && value.length() == 7) ||
        (lastPacket.length() == 20 && lastPacket.startsWith(startBytes3) && value.length() == 7)) {
        incompletePackets = false;
        qDebug() << QStringLiteral("...final bytes received");
        lastPacket.append(value);
        value = lastPacket;
    }

    lastPacket = value;

    if (value.length() != 26) {
        // semaphore for any writing packets (for example, update display)
        if (value.length() == 20 &&
            (value.startsWith(startBytes) || value.startsWith(startBytes2) || value.startsWith(startBytes3))) {
            qDebug() << QStringLiteral("waiting for other bytes...");
            incompletePackets = true;
        }

        qDebug() << QStringLiteral("packet ignored");
        return;
    }

    if (value.at(22) == 0x06) {
        qDebug() << QStringLiteral("start button pressed!");
        requestStart = 1;
    } else if (value.at(22) == 0x07) {
        qDebug() << QStringLiteral("stop button pressed!");
        requestStop = 1;
    }

    /*if ((uint8_t)value.at(1) != 0xbc && value.at(2) != 0x04)  // intense run, these are the bytes for the inclination
       and speed status return;*/

    double speed = GetSpeedFromPacket(value);
    if (!settings.value(QZSettings::kcal_ignore_builtin, QZSettings::default_kcal_ignore_builtin).toBool())
        KCal = GetKcalFromPacket(value);
    else {
        if (watts())
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                                    settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                                    200.0) /
                                (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                                now)))); //(( (0.048* Output in watts +1.19) * body
                                                                            // weight in kg * 3.5) / 200 ) / 60
    }

    if(!firstCharacteristicChanged) {
        Distance += ((speed / (double)3600.0) /
                     ((double)1000.0 / (double)(lastRefreshCharacteristicChanged.msecsTo(now))));
    }

    double distance = GetDistanceFromPacket(value);

    double ucadence = ((uint8_t)value.at(9));
    double cadenceFilter =
        settings.value(QZSettings::domyos_bike_cadence_filter, QZSettings::default_domyos_bike_cadence_filter)
            .toDouble();
    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        if (cadenceFilter == 0 || cadenceFilter > ucadence) {
            Cadence = ucadence;
        } else {
            qDebug() << QStringLiteral("cadence filter out ") << ucadence << cadenceFilter;
        }
    }

    Resistance = value.at(14);
    if (Resistance.value() < 1) {
        qDebug() << QStringLiteral("invalid resistance value ") + QString::number(Resistance.value()) +
                        QStringLiteral(" putting to default");
        Resistance = 1;
    }
    emit resistanceRead(Resistance.value());
    m_pelotonResistance = (Resistance.value() * 100) / max_resistance;

    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            uint8_t heart = ((uint8_t)value.at(18));
            if (heart == 0 || disable_hr_frommachinery) {
                update_hr_from_external();
            } else
                Heart = heart;
        }
    }

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }
    lastRefreshCharacteristicChanged = now;

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

    qDebug() << QStringLiteral("Current speed: ") + QString::number(speed);
    qDebug() << QStringLiteral("Current cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current resistance: ") + QString::number(Resistance.value());
    qDebug() << QStringLiteral("Current heart: ") + QString::number(Heart.value());
    qDebug() << QStringLiteral("Current KCal: ") + QString::number(KCal.value());
    qDebug() << QStringLiteral("Current Distance: ") + QString::number(distance);
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << "QLowEnergyController ERROR!!" << m_control->errorString();
    }

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = speed;
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }
    firstCharacteristicChanged = false;
}

double domyosbike::GetSpeedFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(6) << 8) | ((uint8_t)packet.at(7));
    double data = (double)convertedData / 10.0f;
    return data;
}

double domyosbike::GetKcalFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(10) << 8) | ((uint8_t)packet.at(11));
    if((uint8_t)packet.at(10) == 0xFF)   // sometimes the bike at the startup sends 0xFFFF
        return (double)0;
    return (double)convertedData;
}

double domyosbike::GetDistanceFromPacket(const QByteArray &packet) {
    uint16_t convertedData = (packet.at(12) << 8) | ((uint8_t)packet.at(13));
    double data = ((double)convertedData) / 10.0f;
    return data;
}

void domyosbike::btinit_changyow(bool startTape) {
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

init_reset:
    initPacketRecv = false;
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "init 1 not received, retrying...";
        goto init_reset;
    }

init_data2:
    initPacketRecv = false;
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "init 2 not received, retrying...";
        goto init_data2;
    }

init_start:
    initPacketRecv = false;
    writeCharacteristic(initDataStart, sizeof(initDataStart), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart not received, retrying...";
        goto init_start;
    }

init_start2:
    initPacketRecv = false;
    writeCharacteristic(initDataStart2, sizeof(initDataStart2), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart2 not received, retrying...";
        goto init_start2;
    }

init_start3:
    initPacketRecv = false;
    writeCharacteristic(initDataStart3, sizeof(initDataStart3), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart3 not received, retrying...";
        goto init_start3;
    }

init_start4:
    initPacketRecv = false;
    writeCharacteristic(initDataStart4, sizeof(initDataStart4), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart4 not received, retrying...";
        goto init_start4;
    }

init_start5:
    initPacketRecv = false;
    writeCharacteristic(initDataStart5, sizeof(initDataStart5), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart5 not received, retrying...";
        goto init_start5;
    }

init_start6_7:
    initPacketRecv = false;
    writeCharacteristic(initDataStart6, sizeof(initDataStart6), QStringLiteral("init"), false, false);
    writeCharacteristic(initDataStart7, sizeof(initDataStart7), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart6/7 not received, retrying...";
        goto init_start6_7;
    }

init_start8_9:
    initPacketRecv = false;
    writeCharacteristic(initDataStart8, sizeof(initDataStart8), QStringLiteral("init"), false, false);
    writeCharacteristic(initDataStart9, sizeof(initDataStart9), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart8/9 not received, retrying...";
        goto init_start8_9;
    }

init_start10_11:
    initPacketRecv = false;
    writeCharacteristic(initDataStart10, sizeof(initDataStart10), QStringLiteral("init"), false, false);
    writeCharacteristic(initDataStart11, sizeof(initDataStart11), QStringLiteral("init"), false, true);
    if (!initPacketRecv) {
        qDebug() << "initDataStart10/11 not received, retrying...";
        goto init_start10_11;
    }

    if (startTape) {
init_start12_13:
        initPacketRecv = false;
        writeCharacteristic(initDataStart12, sizeof(initDataStart12), QStringLiteral("init"), false, false);
        writeCharacteristic(initDataStart13, sizeof(initDataStart13), QStringLiteral("init"), false, true);
        if (!initPacketRecv) {
            qDebug() << "initDataStart12/13 not received, retrying...";
            goto init_start12_13;
        }
    }

    initDone = true;
}

void domyosbike::btinit_telink(bool startTape) {
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

void domyosbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
    QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotifyCharacteristic = gattCommunicationChannelService->characteristic(_gattNotifyCharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotifyCharacteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &domyosbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &domyosbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &domyosbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &domyosbike::descriptorWritten);

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotifyCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void domyosbike::searchingStop() { searchStopped = true; }

void domyosbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void domyosbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void domyosbike::serviceScanDone(void) {
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if(!gattCommunicationChannelService) {
        // Main service not found, check if FTMS service is available
        QBluetoothUuid ftmsServiceId((quint16)0x1826);
        QLowEnergyService *ftmsService = m_control->createServiceObject(ftmsServiceId);
        if(ftmsService) {
            QSettings settings;
            settings.setValue(QZSettings::ftms_bike, bluetoothDevice.name());
            qDebug() << "forcing FTMS bike since it has FTMS service but not the main domyos service";
            if(homeform::singleton())
                homeform::singleton()->setToastRequested("FTMS bike found, restart the app to apply the change");
            delete ftmsService;
        }
        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &domyosbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void domyosbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("domyosbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();

    m_control->disconnectFromDevice();
}

void domyosbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("domyosbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void domyosbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
                    device.address().toString() + ')';
    if (device.name().startsWith(QStringLiteral("Domyos-Bike")) &&
        !device.name().startsWith(QStringLiteral("DomyosBridge"))) {
        bluetoothDevice = device;

        if (device.address().toString().startsWith(QStringLiteral("57"))) {
            qDebug() << QStringLiteral("domyos telink bike found");
            bike_type = TELINK;
        } else {
            qDebug() << QStringLiteral("domyos changyow bike found");
            bike_type = CHANG_YOW;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &domyosbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &domyosbike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &domyosbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &domyosbike::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    searchStopped = false;
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
            searchStopped = false;
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool domyosbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

resistance_t domyosbike::pelotonToBikeResistance(int pelotonResistance) {
    return (pelotonResistance * max_resistance) / 100;
}

resistance_t domyosbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << currentCadence().value();

    for (resistance_t i = 1; i < max_resistance; i++) {
        if (wattsFromResistance(i) <= power && wattsFromResistance(i + 1) >= power) {
            return i;
        }
    }
    if (power < wattsFromResistance(1))
        return 1;
    else
        return max_resistance;
}

uint16_t domyosbike::wattsFromResistance(double resistance) {
    QSettings settings;
    if (settings.value(QZSettings::domyos_bike_500_profile_v2, QZSettings::default_domyos_bike_500_profile_v2).toBool()) {
        switch ((int)resistance) {
        case 1:
            return (5.0 * Cadence.value()) / 9.5488;
        case 2:
            return (5.7 * Cadence.value()) / 9.5488;
        case 3:
            return (6.5 * Cadence.value()) / 9.5488;
        case 4:
            return (7.5 * Cadence.value()) / 9.5488;
        case 5:
            return (8.6 * Cadence.value()) / 9.5488;
        case 6:
            return (9.9 * Cadence.value()) / 9.5488;
        case 7:
            return (11.4 * Cadence.value()) / 9.5488;
        case 8:
            return (13.6 * Cadence.value()) / 9.5488;
        case 9:
            return (15.3 * Cadence.value()) / 9.5488;
        case 10:
            return (17.3 * Cadence.value()) / 9.5488;
        case 11:
            return (19.8 * Cadence.value()) / 9.5488;
        case 12:
            return (22.5 * Cadence.value()) / 9.5488;
        case 13:
            return (25.6 * Cadence.value()) / 9.5488;
        case 14:
            return (28.4 * Cadence.value()) / 9.5488;
        case 15:
            return (35.9 * Cadence.value()) / 9.5488;
        }
        return 0;
    } else if (!settings.value(QZSettings::domyos_bike_500_profile_v1, QZSettings::default_domyos_bike_500_profile_v1)
             .toBool() ||
        resistance < 8)
        return ((10.39 + 1.45 * (resistance - 1.0)) * (exp(0.028 * (currentCadence().value()))));
    else {
        switch ((int)resistance) {
        case 8:
            return (13.6 * Cadence.value()) / 9.5488;
        case 9:
            return (15.3 * Cadence.value()) / 9.5488;
        case 10:
            return (17.3 * Cadence.value()) / 9.5488;
        case 11:
            return (19.8 * Cadence.value()) / 9.5488;
        case 12:
            return (22.5 * Cadence.value()) / 9.5488;
        case 13:
            return (25.6 * Cadence.value()) / 9.5488;
        case 14:
            return (28.4 * Cadence.value()) / 9.5488;
        case 15:
            return (35.9 * Cadence.value()) / 9.5488;
        }
        return ((10.39 + 1.45 * (resistance - 1.0)) * (exp(0.028 * (currentCadence().value()))));
    }
}

uint16_t domyosbike::watts() {
    double v = 0;
    // const resistance_t max_resistance = 15;
    // ref
    // https://translate.google.com/translate?hl=it&sl=en&u=https://support.wattbike.com/hc/en-us/articles/115001881825-Power-Resistance-and-Cadence-Tables&prev=search&pto=aue

    if (currentCadence().value() <= 0) {
        return 0;
    }
    v = wattsFromResistance(currentResistance().value());
    return v;
}

void domyosbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
