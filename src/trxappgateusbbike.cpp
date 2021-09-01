#include "trxappgateusbbike.h"
#include "keepawakehelper.h"
#include "virtualbike.h"
#include <QBluetoothLocalDevice>

#include <QDateTime>

#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

trxappgateusbbike::trxappgateusbbike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &trxappgateusbbike::update);
    refresh->start(200ms);
}

void trxappgateusbbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                            bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &trxappgateusbbike::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic,
                                                         QByteArray((const char *)data, data_len));

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void trxappgateusbbike::forceResistance(int8_t requestResistance) {

    uint8_t resistance[] = {0xf0, 0xa6, 0x01, 0x01, 0x00, 0x00};
    if (bike_type == DKN_MOTION_2) {
        resistance[2] = 0x02;
    }
    resistance[4] = requestResistance + 1;
    for (uint8_t i = 0; i < sizeof(resistance) - 1; i++) {

        resistance[5] += resistance[i]; // the last byte is a sort of a checksum
    }
    writeCharacteristic((uint8_t *)resistance, sizeof(resistance),
                        QStringLiteral("resistance ") + QString::number(requestResistance), false, true);
}

void trxappgateusbbike::update() {
    // qDebug() << bike.isValid() << m_control->state() << gattCommunicationChannelService <<
    // gattWriteCharacteristic.isValid() << gattNotifyCharacteristic.isValid() << initDone;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        btinit(false);
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&

               gattNotify1Characteristic.isValid() &&
               // gattNotify2Characteristic.isValid() &&
               initDone) {

        update_metrics(false, 0);

        // updating the bike console every second
        if (sec1update++ == (1000 / refresh->interval())) {

            sec1update = 0;
            // updateDisplay(elapsed);
        }

        QSettings settings;
        bool toorx30 = settings.value(QStringLiteral("toorx_3_0"), false).toBool();
        if (toorx30 == false && (bike_type == TYPE::IRUNNING || bike_type == TYPE::ICONSOLE)) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x01, 0x01, 0x94};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::DKN_MOTION || bike_type == TYPE::DKN_MOTION_2) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x02, 0x01, 0x95};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::CHANGYOW) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x23, 0x01, 0xb6};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::ASVIVA) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x00, 0xc8, 0x5a};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::JLL_IC400) {

            static unsigned char pollCounter = 0x0b;
            static unsigned char crc = 0x59;
            uint8_t noOpData[] = {0xf0, 0xa2, 0x00, 0xc8, 0x00};
            noOpData[2] = pollCounter;
            crc += 0x0c;
            noOpData[4] = crc;
            pollCounter += 0x0c;
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x23, 0xd3, 0x88};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        }
        if (requestResistance != -1) {
            if (requestResistance > 32) {
                requestResistance = 32;
            } else if (requestResistance < 1) {
                requestResistance = 1;
            }
            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));

                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
    }
}

void trxappgateusbbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void trxappgateusbbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    double heart = 0;
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    emit packetReceived();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;
    if ((newValue.length() != 21 && (bike_type != JLL_IC400 && bike_type != ASVIVA)) ||
        (newValue.length() != 19 && (bike_type == JLL_IC400 || bike_type == ASVIVA))) {
        return;
    }

    /*
    if(bike_type == TYPE::IRUNNING)
    {
        if(newValue.at(15) == 0x03 && newValue.at(16) == 0x02 && readyToStart == false)
        {
            readyToStart = true;
            requestStart = 1;
        }
    }
    else
    {
        if(newValue.at(16) == 0x04 && newValue.at(17) == 0x03 && readyToStart == false)
        {
            readyToStart = true;
            requestStart = 1;
        }
    }
*/

    double cadence = GetCadenceFromPacket(newValue);
    double speed = 0.0;
    double resistance = 0.0;
    double kcal = 0.0;
    double watt = 0.0;
    if (bike_type != JLL_IC400 && bike_type != ASVIVA) {

        speed = GetSpeedFromPacket(newValue);
        resistance = GetResistanceFromPacket(newValue);
        kcal = GetKcalFromPacket(newValue);
        watt = GetWattFromPacket(newValue);
    } else {

        speed = cadence * 0.37407407407407407407407407407407;
        if (Heart.value() > 0) {
            int avgP = ((settings.value(QStringLiteral("power_hr_pwr1"), 200).toDouble() *
                         settings.value(QStringLiteral("power_hr_hr2"), 170).toDouble()) -
                        (settings.value(QStringLiteral("power_hr_pwr2"), 230).toDouble() *
                         settings.value(QStringLiteral("power_hr_hr1"), 150).toDouble())) /
                           (settings.value(QStringLiteral("power_hr_hr2"), 170).toDouble() -
                            settings.value(QStringLiteral("power_hr_hr1"), 150).toDouble()) +
                       (Heart.value() * ((settings.value(QStringLiteral("power_hr_pwr1"), 200).toDouble() -
                                          settings.value(QStringLiteral("power_hr_pwr2"), 230).toDouble()) /
                                         (settings.value(QStringLiteral("power_hr_hr1"), 150).toDouble() -
                                          settings.value(QStringLiteral("power_hr_hr2"), 170).toDouble())));
            if (Speed.value() > 0) {
                watt = avgP;
            } else {
                watt = 0;
            }

            kcal = KCal.value() + ((((0.048 * ((double)watts()) + 1.19) *
                                     settings.value(QStringLiteral("weight"), 75.0).toFloat() * 3.5) /
                                    200.0) /
                                   (60000.0 / ((double)lastTimeCharChanged.msecsTo(
                                                  QTime::currentTime())))); //(( (0.048* Output in watts +1.19) *
                                                                            // body weight in kg * 3.5) / 200 ) / 60
        }
    }

#ifdef Q_OS_ANDROID
    if (settings.value(QStringLiteral("ant_heart"), false).toBool()) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
    } else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            if (bike_type != JLL_IC400 && bike_type != ASVIVA) {

                heart = ((uint8_t)(newValue.at(15)) - 1);
            } else {
                heart = ((uint8_t)(newValue.at(17))) + (((uint8_t)(newValue.at(16))) * 83);
            }
            if (heart == 0.0 || settings.value(QStringLiteral("heart_ignore_builtin"), false).toBool()) {

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
                lockscreen h;
                long appleWatchHeartRate = h.heartRate();
                h.setKcal(KCal.value());
                h.setDistance(Distance.value());
                Heart = appleWatchHeartRate;
                debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
#endif
#endif
            } else {

                Heart = heart;
            }
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cad = settings.value("bike_cadence_sensor", false).toBool();
    bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
    if (ios_peloton_workaround && cad && h && firstVirtualBike) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    FanSpeed = 0;

    if (!firstCharChanged) {
        DistanceCalculated += ((speed / 3600.0) / (1000.0 / (lastTimeCharChanged.msecsTo(QTime::currentTime()))));
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(speed));
    emit debug(QStringLiteral("Current cadence: ") + QString::number(cadence));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(kcal));
    emit debug(QStringLiteral("Current watt: ") + QString::number(watt));
    emit debug(QStringLiteral("Current Elapsed from the bike (not used): ") +
               QString::number(GetElapsedFromPacket(newValue)));
    emit debug(QStringLiteral("Current Elapsed: ") + QString::number(elapsed.value()));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(DistanceCalculated));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (!settings.value(QStringLiteral("speed_power_based"), false).toBool()) {
        Speed = speed;
    } else {
        Speed = metric::calculateSpeedFromPower(m_watt.value());
    }
    KCal = kcal;
    Distance = DistanceCalculated;
    if (settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled"))
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = cadence;
    }
    if (settings.value(QStringLiteral("power_sensor_name"), QStringLiteral("Disabled"))
            .toString()
            .startsWith(QStringLiteral("Disabled")))
        m_watt = watt;

    double ac = 0.01243107769;
    double bc = 1.145964912;
    double cc = -23.50977444;

    double ar = 0.1469553975;
    double br = -5.841344538;
    double cr = 97.62165482;

    m_pelotonResistance =
        (((sqrt(pow(br, 2.0) -
                4.0 * ar *
                    (cr - (m_watt.value() * 132.0 / (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
           br) /
          (2.0 * ar)) *
         settings.value(QStringLiteral("peloton_gain"), 1.0).toDouble()) +
        settings.value(QStringLiteral("peloton_offset"), 0.0).toDouble();
    if (bike_type == JLL_IC400 || bike_type == ASVIVA) {
        resistance = m_pelotonResistance.value();
    }

    Resistance = resistance;
    emit resistanceRead(Resistance.value());

    emit debug(QStringLiteral("Current resistance: ") + QString::number(resistance));

    lastTimeCharChanged = QTime::currentTime();
    firstCharChanged = false;
}

uint16_t trxappgateusbbike::GetElapsedFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(4) - 1);
    convertedData += ((packet.at(5) - 1) * 60);
    return convertedData;
}

double trxappgateusbbike::GetSpeedFromPacket(const QByteArray &packet) {

    uint16_t convertedData = (packet.at(7) - 1) + ((packet.at(6) - 1) * 100);
    double data = (double)(convertedData) / 10.0f;
    return data;
}

double trxappgateusbbike::GetKcalFromPacket(const QByteArray &packet) {

    uint16_t convertedData = ((packet.at(12) - 1) * 100) + (packet.at(13) - 1);
    return (double)(convertedData);
}

double trxappgateusbbike::GetWattFromPacket(const QByteArray &packet) {

    uint16_t convertedData = ((packet.at(16) - 1) * 100) + (packet.at(17) - 1);
    double data = ((double)(convertedData)) / 10.0f;
    return data;
}

double trxappgateusbbike::GetCadenceFromPacket(const QByteArray &packet) {

    uint16_t convertedData;
    if (bike_type != JLL_IC400 && bike_type != ASVIVA) {
        convertedData = (packet.at(9) - 1) + ((packet.at(8) - 1) * 100);
    } else {
        convertedData = ((uint16_t)packet.at(9)) + ((uint16_t)packet.at(8) * 100);
    }
    double data = (convertedData);
    if (data < 0) {
        return 0;
    }
    return data;
}

double trxappgateusbbike::GetResistanceFromPacket(const QByteArray &packet) {

    uint16_t convertedData = packet.at(18);
    double data = (convertedData - 1);
    if (data < 0) {
        return 0;
    }
    return data;
}

void trxappgateusbbike::btinit(bool startTape) {

    Q_UNUSED(startTape);
    QSettings settings;
    bool toorx30 = settings.value(QStringLiteral("toorx_3_0"), false).toBool();

    if (toorx30 == false && (bike_type == TYPE::IRUNNING || bike_type == TYPE::ICONSOLE)) {

        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa1, 0x01, 0x01, 0x93};
        const uint8_t initData3[] = {0xf0, 0xa3, 0x01, 0x01, 0x01, 0x96};
        const uint8_t initData4[] = {0xf0, 0xa5, 0x01, 0x01, 0x02, 0x99};
        const uint8_t initData5[] = {0xf0, 0xa6, 0x01, 0x01, 0x06, 0x9e};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING) {
            QThread::msleep(400);
        }
    } else if (bike_type == TYPE::DKN_MOTION || bike_type == TYPE::DKN_MOTION_2) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x02, 0x01, 0x93};
        const uint8_t initData3[] = {0xf0, 0xa3, 0x02, 0x01, 0x01, 0x97};
        const uint8_t initData4[] = {0xf0, 0xa5, 0x02, 0x01, 0x02, 0x9a};
        const uint8_t initData5[] = {0x40, 0x00, 0x9a, 0x46, 0x20};
        const uint8_t initData6[] = {0xf0, 0xa6, 0x02, 0x01, 0x02, 0x9b};
        const uint8_t noOpData[] = {0xf0, 0xa2, 0x02, 0x01, 0x95};
        const uint8_t initData7[] = {0xf0, 0xa6, 0x02, 0x01, 0x03, 0x9c};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    } else if (bike_type == TYPE::CHANGYOW) {

        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x23, 0x01, 0xb5};
        const uint8_t initData4[] = {0xf0, 0xa3, 0x23, 0x01, 0x01, 0xb8};
        const uint8_t initData5[] = {0xf0, 0xa5, 0x23, 0x01, 0x02, 0xbb};
        const uint8_t initData6[] = {0xf0, 0xa6, 0x23, 0x01, 0x06, 0xc0};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (bike_type == TYPE::JLL_IC400) {

        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x03, 0xc9, 0x5c};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x00, 0xc8, 0x59};
        const uint8_t initData4[] = {0xf0, 0xa0, 0x01, 0xc9, 0x5a};
        const uint8_t initData5[] = {0xf0, 0xa1, 0x05, 0xc8, 0x5e};
        const uint8_t initData6[] = {0xf0, 0xa2, 0x22, 0xc8, 0x7c};
        const uint8_t initData7[] = {0xf0, 0xa0, 0x39, 0xc9, 0x92};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (bike_type == TYPE::ASVIVA) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x01, 0xc9, 0x5a};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x00, 0xc8, 0x59};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x00, 0xc8, 0x59};
        const uint8_t initData5[] = {0xf0, 0xa2, 0x00, 0xc8, 0x5a};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else {

        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa5, 0x23, 0xd3, 0x04, 0x8f};
        const uint8_t initData3[] = {0xf0, 0xa0, 0x23, 0xd3, 0x86};
        const uint8_t initData4[] = {0xf0, 0xa1, 0x23, 0xd3, 0x87};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x23, 0xd3, 0x01, 0x15, 0x01, 0x02, 0x51, 0x01, 0x51, 0x45};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x23, 0xd3, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x95};
        const uint8_t initData7[] = {0xf0, 0xaf, 0x23, 0xd3, 0x02, 0x97};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    }
    initDone = true;
}

void trxappgateusbbike::stateChanged(QLowEnergyService::ServiceState state) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        auto characteristics_list = gattCommunicationChannelService->characteristics();
        for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
            emit debug(QStringLiteral("characteristic ") + c.uuid().toString());
        }

        QString uuidWrite = QStringLiteral("0000fff2-0000-1000-8000-00805f9b34fb");
        QString uuidNotify1 = QStringLiteral("0000fff1-0000-1000-8000-00805f9b34fb");
        QString uuidNotify2 = QStringLiteral("49535343-4c8a-39b3-2f49-511cff073b7e");

        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW || bike_type == TYPE::ICONSOLE ||
            bike_type == TYPE::JLL_IC400 || bike_type == TYPE::DKN_MOTION_2) {
            uuidWrite = QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3");
            uuidNotify1 = QStringLiteral("49535343-1E4D-4BD9-BA61-23C647249616");
            uuidNotify2 = QStringLiteral("49535343-4c8a-39b3-2f49-511cff073b7e");
        }

        QBluetoothUuid _gattWriteCharacteristicId(uuidWrite);
        QBluetoothUuid _gattNotify1CharacteristicId(uuidNotify1);
        QBluetoothUuid _gattNotify2CharacteristicId(uuidNotify2);

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW) {
            gattNotify2Characteristic = gattCommunicationChannelService->characteristic(_gattNotify2CharacteristicId);
        }

        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW) {
            Q_ASSERT(gattNotify2Characteristic.isValid());
        }

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &trxappgateusbbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &trxappgateusbbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &trxappgateusbbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &trxappgateusbbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstVirtualBike && !virtualBike) {

            QSettings settings;
            bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));

                virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
                // connect(virtualBike,&virtualbike::debug ,this,&trxappgateusbbike::debug);
            }
        }
        firstVirtualBike = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW) {
            gattCommunicationChannelService->writeDescriptor(
                gattNotify2Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
        }
    }
}

void trxappgateusbbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void trxappgateusbbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void trxappgateusbbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QString uuid = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
    QString uuid2 = QStringLiteral("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
    if (bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW || bike_type == TYPE::ICONSOLE ||
        bike_type == TYPE::JLL_IC400) {
        uuid = uuid2;
    }

    if (bike_type == DKN_MOTION) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = DKN_MOTION_2;
            uuid = uuid2;
        }
    }

    QBluetoothUuid _gattCommunicationChannelServiceId(uuid);
    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);

    if (gattCommunicationChannelService == nullptr) {
        qDebug() << QStringLiteral("invalid service") << uuid;

        return;
    }

    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &trxappgateusbbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void trxappgateusbbike::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("trxappgateusbbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbbike::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("trxappgateusbbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void trxappgateusbbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    QSettings settings;
    bool JLL_IC400_bike = settings.value(QStringLiteral("jll_IC400_bike"), false).toBool();
    bool ASVIVA_bike = settings.value(QStringLiteral("asviva_bike"), false).toBool();
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    // if(device.name().startsWith("TOORX") || device.name().startsWith("V-RUN") || device.name().startsWith("FS-")
    // || device.name().startsWith("i-Console+") || device.name().startsWith("i-Running"))
    {
        // if(device.name().startsWith("i-Running") || device.name().startsWith("i-Console+"))
        bike_type = TYPE::IRUNNING;
        /*else
            bike_type = TYPE::TRXAPPGATE;*/

        if (JLL_IC400_bike) {
            refresh->start(500ms);

            bike_type = TYPE::JLL_IC400;
            qDebug() << QStringLiteral("JLL_IC400 bike found");
        } else if (ASVIVA_bike) {
            refresh->start(500ms);

            bike_type = TYPE::ASVIVA;
            qDebug() << QStringLiteral("ASVIVA bike found");
        } else if (device.address().toString().toUpper().startsWith(QStringLiteral("E8"))) {

            bike_type = TYPE::CHANGYOW;
            qDebug() << QStringLiteral("CHANGYOW bike found");
        } else if (device.name().toUpper().startsWith(QStringLiteral("BFCP"))) {

            bike_type = TYPE::SKANDIKAWIRY;
            qDebug() << QStringLiteral("SKANDIKAWIRY bike found");
        } else if (device.name().toUpper().startsWith(QStringLiteral("ICONSOLE+"))) {

            bike_type = TYPE::ICONSOLE;
            qDebug() << QStringLiteral("ICONSOLE bike found");
        } else if (device.name().toUpper().startsWith(QStringLiteral("DKN MOTION"))) {

            bike_type = TYPE::DKN_MOTION;
            qDebug() << QStringLiteral("DKN MOTION bike found");
        }

        bluetoothDevice = device;
        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &trxappgateusbbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &trxappgateusbbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &trxappgateusbbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &trxappgateusbbike::controllerStateChanged);

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

uint16_t trxappgateusbbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

bool trxappgateusbbike::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *trxappgateusbbike::VirtualBike() { return virtualBike; }

void *trxappgateusbbike::VirtualDevice() { return VirtualBike(); }

double trxappgateusbbike::odometer() { return DistanceCalculated; }

void trxappgateusbbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}
