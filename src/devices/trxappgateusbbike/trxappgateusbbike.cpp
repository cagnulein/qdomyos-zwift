#include "trxappgateusbbike.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>

#include <QDateTime>

#include <QEventLoop>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>

using namespace std::chrono_literals;

trxappgateusbbike::trxappgateusbbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                                     double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
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

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse)
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    else
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();

    if (timeout.isActive() == false) {
        emit debug(QStringLiteral(" exit for timeout"));
    }
}

void trxappgateusbbike::forceResistance(resistance_t requestResistance) {

    if (!initDone) {
        emit debug(QStringLiteral("forceResistance Ignored, init not done yet"));
        return;
    }

    uint8_t resistance[] = {0xf0, 0xa6, 0x01, 0x01, 0x00, 0x00};
    if (bike_type == DKN_MOTION_2) {
        resistance[2] = 0x02;
    } else if (bike_type == VIRTUFIT || bike_type == VIRTUFIT_2) {
        resistance[2] = 0x1e;
    } else if (bike_type == HOP_SPORT_HS_090H || bike_type == HOP_SPORT_HS_090H_2) {
        resistance[2] = 0x3f;
    } else if (bike_type == TOORX_SRX_500) {
        resistance[2] = 0x23;
    } else if (bike_type == FAL_SPORTS) {
        resistance[2] = 0x1b;
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
        bool toorx30 = settings.value(QZSettings::toorx_3_0, QZSettings::default_toorx_3_0).toBool();
        if (toorx30 == false &&
            (bike_type == TYPE::IRUNNING || bike_type == TYPE::IRUNNING_2 || bike_type == TYPE::ICONSOLE || bike_type == TYPE::ICONSOLE_2 ||
             bike_type == TYPE::HERTZ_XR_770 || bike_type == TYPE::HERTZ_XR_770_2 || bike_type == PASYOU)) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x01, 0x01, 0x94};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::DKN_MOTION || bike_type == TYPE::DKN_MOTION_2) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x02, 0x01, 0x95};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::CASALL) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x3b, 0x01, 0xce};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::FAL_SPORTS) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x1b, 0x01, 0xae};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::TUNTURI || bike_type == TYPE::TUNTURI_2) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x03, 0x01, 0x96};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::CHANGYOW || bike_type == TYPE::ENERFIT_SPX_9500 || bike_type == TYPE::ENERFIT_SPX_9500_2 || bike_type == HAMMER_SPEED_BIKE_S) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x23, 0x01, 0xb6};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::ASVIVA || bike_type == TYPE::FYTTER_RI08) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x00, 0xc8, 0x5a};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::VIRTUFIT || bike_type == VIRTUFIT_2) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x1e, 0x01, 0xb1};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::REEBOK || bike_type == REEBOK_2) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x32, 0x01, 0xc5};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::HOP_SPORT_HS_090H || bike_type == TYPE::HOP_SPORT_HS_090H_2) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x3f, 0x01, 0xd2};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::TOORX_SRX_500) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x23, 0x01, 0xb6};
            writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
        } else if (bike_type == TYPE::BIKZU) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x54, 0x31, 0x17};
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
        } else if (bike_type == TYPE::TAURUA_IC90) {

            const uint8_t noOpData[] = {0xf0, 0xa2, 0x01, 0x31, 0xc4};
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
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    emit packetReceived();

    qDebug() << newValue.toHex(' ') << bike_type;

    lastPacket = newValue;
    if ((newValue.length() != 21 && (bike_type != JLL_IC400 && bike_type != ASVIVA && bike_type != FYTTER_RI08 &&
                                     bike_type != TUNTURI && bike_type != TUNTURI_2 && bike_type != TOORX_SRX_500 &&
                                     bike_type != FAL_SPORTS && bike_type != HAMMER_SPEED_BIKE_S)) ||
        (newValue.length() != 19 && (bike_type == JLL_IC400 || bike_type == ASVIVA || bike_type == FYTTER_RI08 ||
                                     bike_type == PASYOU || bike_type == HAMMER_SPEED_BIKE_S)) ||
        (newValue.length() != 20 && newValue.length() != 21 &&
         (bike_type == TUNTURI || bike_type == TYPE::TUNTURI_2))) {
        return;
    }

    if (bike_type == TOORX_SRX_500 && newValue.length() < 19) {
        qDebug() << QStringLiteral("ignoring short packet ") << newValue.length();
        return;
    }

    double cadence = GetCadenceFromPacket(newValue);
    double speed = 0.0;
    double resistance = 0.0;
    double watt = 0.0;
    QTime now = QTime::currentTime();
    if (bike_type == FYTTER_RI08) {
        speed = cadence * 0.37407407407407407407407407407407;
        watt = GetWattFromPacketFytter(newValue);
        if (watt)
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                                     settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                                    200.0) /
                                   (60000.0 / ((double)lastTimeCharChanged.msecsTo(
                                                  now)))); //(( (0.048* Output in watts +1.19) *
                                                                            // body weight in kg * 3.5) / 200 ) / 60
    } else if (bike_type == TUNTURI || bike_type == TUNTURI_2 || bike_type == FAL_SPORTS) {
        speed = cadence * 0.37407407407407407407407407407407;
        resistance = GetResistanceFromPacket(newValue);
        watt = GetWattFromPacket(newValue);
        if (watt)
            KCal += ((((0.048 * ((double)watts()) + 1.19) *
                                     settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                                    200.0) /
                                   (60000.0 / ((double)lastTimeCharChanged.msecsTo(
                                                  now)))); //(( (0.048* Output in watts +1.19) *
                                                                            // body weight in kg * 3.5) / 200 ) / 60
    } else if (bike_type != JLL_IC400 && bike_type != ASVIVA && bike_type != HAMMER_SPEED_BIKE_S) {

        speed = GetSpeedFromPacket(newValue);
        resistance = GetResistanceFromPacket(newValue);
        watt = GetWattFromPacket(newValue);
        if (!settings.value(QZSettings::kcal_ignore_builtin, QZSettings::default_kcal_ignore_builtin).toBool())
            KCal = GetKcalFromPacket(newValue);
        else {
            if (watt)
                KCal += ((((0.048 * ((double)watts()) + 1.19) *
                                      settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                                     200.0) /
                                    (60000.0 / ((double)lastTimeCharChanged.msecsTo(
                                                   now)))); //(( (0.048* Output in watts +1.19) * body
                                                                             // weight in kg * 3.5) / 200 ) / 60
        }
    } else {

        speed = cadence * 0.37407407407407407407407407407407;
        if (Heart.value() > 0) {
            watt = wattFromHR(true);

            if (watt)
                KCal += ((((0.048 * ((double)watt) + 1.19) *
                                      settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                                     200.0) /
                                    (60000.0 / ((double)lastTimeCharChanged.msecsTo(
                                                   now)))); //(( (0.048* Output in watts +1.19) *
                                                                             // body weight in kg * 3.5) / 200 ) / 60
        }
    }

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = (uint8_t)KeepAwakeHelper::heart();
    } else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            if (bike_type != JLL_IC400 && bike_type != ASVIVA && bike_type != HAMMER_SPEED_BIKE_S) {

                heart = ((uint8_t)(newValue.at(15)) - 1) + ((uint8_t)((newValue.at(14)) - 1) * 100); // #454
            } else {
                heart = ((uint8_t)(newValue.at(17))) + (((uint8_t)(newValue.at(16))) * 83);
            }
            if (heart == 0.0 ||
                settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool()) {
                update_hr_from_external();
            } else {
                Heart = heart;
            }
        }
    }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cad = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cad && h && firstVirtualBike) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }

    if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
        Speed = speed;
    } else {
        Speed = metric::calculateSpeedFromPower(
            watts(), Inclination.value(), Speed.value(),
            fabs(QDateTime::currentDateTime().msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }
    if (!firstCharChanged) {
        Distance += ((Speed.value() / 3600.0) / (1000.0 / (lastTimeCharChanged.msecsTo(now))));
    }
    if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled"))) {
        Cadence = cadence;
    }

    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
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
         settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
        settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
    if (bike_type == JLL_IC400 || bike_type == ASVIVA || bike_type == FYTTER_RI08) {
        resistance = m_pelotonResistance.value();
    }

    if (ResistanceFromFTMSAccessoryLastTime == 0) {
        Resistance = resistance;
        emit resistanceRead(Resistance.value());
    } else {
        Resistance = ResistanceFromFTMSAccessory.value();
    }

    emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current cadence: ") + QString::number(cadence));
    emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
    emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
    emit debug(QStringLiteral("Current watt: ") + QString::number(watt));
    emit debug(QStringLiteral("Current Elapsed from the bike (not used): ") +
               QString::number(GetElapsedFromPacket(newValue)));
    emit debug(QStringLiteral("Current Elapsed: ") + QString::number(elapsed.value()));
    emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current resistance: ") + QString::number(resistance));

    lastTimeCharChanged = now;
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

double trxappgateusbbike::GetWattFromPacketFytter(const QByteArray &packet) {

    if (!packet.at(6))
        return 0;
    uint16_t convertedData = ((packet.at(6) - 1) * 100) + (packet.at(7));
    double data = ((double)(convertedData));
    return data;
}

double trxappgateusbbike::GetWattFromPacket(const QByteArray &packet) {

    uint16_t convertedData = ((packet.at(16) - 1) * 100) + (packet.at(17) - 1);
    double data = ((double)(convertedData)) / 10.0f;
    return data;
}

double trxappgateusbbike::GetCadenceFromPacket(const QByteArray &packet) {

    QSettings settings;
    double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
    double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();

    uint16_t convertedData;
    if (bike_type != JLL_IC400 && bike_type != ASVIVA && bike_type != FYTTER_RI08 && bike_type != HAMMER_SPEED_BIKE_S) {
        convertedData = (packet.at(9) - 1) + ((packet.at(8) - 1) * 100);
    } else {
        convertedData = ((uint16_t)packet.at(9)) + ((uint16_t)packet.at(8) * 100);
    }
    double data = (convertedData);
    if (data < 0) {
        return 0;
    }
    return (data * cadence_gain) + cadence_offset   ;
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
    bool toorx30 = settings.value(QZSettings::toorx_3_0, QZSettings::default_toorx_3_0).toBool();

    if (toorx30 == false &&
        (bike_type == TYPE::IRUNNING || bike_type == TYPE::IRUNNING_2 || bike_type == TYPE::ICONSOLE || bike_type == TYPE::ICONSOLE_2 || bike_type == PASYOU)) {

        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa1, 0x01, 0x01, 0x93};
        const uint8_t initData3[] = {0xf0, 0xa3, 0x01, 0x01, 0x01, 0x96};
        const uint8_t initData4[] = {0xf0, 0xa5, 0x01, 0x01, 0x02, 0x99};
        const uint8_t initData5[] = {0xf0, 0xa6, 0x01, 0x01, 0x06, 0x9e};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::IRUNNING_2 || bike_type == PASYOU) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::IRUNNING_2 || bike_type == PASYOU) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::IRUNNING_2 || bike_type == PASYOU) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::IRUNNING_2 || bike_type == PASYOU) {
            QThread::msleep(400);
        }
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        if (bike_type == TYPE::IRUNNING || bike_type == TYPE::IRUNNING_2 || bike_type == PASYOU) {
            QThread::msleep(400);
        }
    } else if (bike_type == TYPE::HOP_SPORT_HS_090H || bike_type == TYPE::HOP_SPORT_HS_090H_2) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x00, 0x91};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x3f, 0x01, 0xd0};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x3f, 0x01, 0xd1};
        const uint8_t initData4[] = {0xf0, 0xa3, 0x3f, 0x01, 0x01, 0xd4};
        const uint8_t initData5[] = {0xf0, 0xa4, 0x3f, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xde};
        const uint8_t initData6[] = {0xf0, 0xa5, 0x3f, 0x01, 0x02, 0xd7};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    } else if (bike_type == TYPE::TOORX_SRX_500) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x00, 0x91};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x23, 0x01, 0xb5};
        const uint8_t initData4[] = {0xf0, 0xa0, 0x23, 0x01, 0xb4};
        const uint8_t initData5[] = {0xf0, 0xa3, 0x23, 0x01, 0x01, 0xb8};
        const uint8_t initData6[] = {0xf0, 0xa4, 0x23, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xc2};
        const uint8_t initData7[] = {0xf0, 0xa5, 0x23, 0x01, 0x02, 0xbb};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    } else if (bike_type == TYPE::VIRTUFIT || bike_type == VIRTUFIT_2) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x1e, 0x01, 0xaf};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x1e, 0x01, 0xb0};
        const uint8_t initData4[] = {0xf0, 0xa3, 0x1e, 0x01, 0x01, 0xb3};
        const uint8_t initData5[] = {0xf0, 0xa4, 0x1e, 0x01, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xbd};
        const uint8_t initData6[] = {0xf0, 0xa5, 0x1e, 0x01, 0x02, 0xb6};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    } else if (bike_type == TYPE::HERTZ_XR_770 || bike_type == TYPE::HERTZ_XR_770_2 || bike_type == TYPE::FITHIWAY) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x02, 0x01, 0x93};
        const uint8_t initData3[] = {0xf0, 0xa3, 0x01, 0x01, 0x01, 0x96};
        const uint8_t initData4[] = {0xf0, 0xa4, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xa0};
        const uint8_t initData5[] = {0xf0, 0xa5, 0x01, 0x01, 0x02, 0x99};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
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
    } else if (bike_type == TYPE::CHANGYOW || bike_type == TYPE::ENERFIT_SPX_9500 || bike_type == TYPE::ENERFIT_SPX_9500_2 || bike_type == HAMMER_SPEED_BIKE_S) {

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
    } else if (bike_type == TYPE::REEBOK || bike_type == TYPE::REEBOK_2) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x32, 0x01, 0xc3};
        const uint8_t initData2[] = {0xf0, 0xa3, 0x32, 0x01, 0x01, 0xc7};
        const uint8_t initData3[] = {0xf0, 0xa4, 0x32, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xd1};
        const uint8_t initData4[] = {0xf0, 0xa5, 0x32, 0x01, 0x02, 0xca};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);

        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);

        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (bike_type == TYPE::TUNTURI || bike_type == TYPE::TUNTURI_2) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x03, 0x01, 0x94};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x03, 0x01, 0x95};
        const uint8_t initData4[] = {0xf0, 0xa3, 0x03, 0x01, 0x01, 0x98};
        const uint8_t initData5[] = {0xf0, 0xa5, 0x03, 0x01, 0x02, 0x9b};
        const uint8_t initData6[] = {0xf0, 0xa6, 0x03, 0x01, 0x06, 0xa0};
        const uint8_t initData7[] = {0xf0, 0xa6, 0x03, 0x01, 0x09, 0xa3};
        const uint8_t noOpData[] = {0xf0, 0xa2, 0x03, 0x01, 0x96};

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
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp-init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp-init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp-init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)noOpData, sizeof(noOpData), QStringLiteral("noOp-init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (bike_type == TYPE::BIKZU) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x54, 0x31, 0x15};
        const uint8_t initData2[] = {0xf0, 0xa3, 0x54, 0x31, 0x01, 0x19};
        const uint8_t initData3[] = {0xf0, 0xa4, 0x54, 0x31, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x23};
        const uint8_t initData4[] = {0xf0, 0xa5, 0x54, 0x31, 0x02, 0x1c};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (bike_type == TYPE::FAL_SPORTS) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x01, 0x92};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x1b, 0x01, 0xac};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x1b, 0x01, 0xad};
        const uint8_t initData4[] = {0xf0, 0xa3, 0x1b, 0x01, 0x01, 0xb0};
        const uint8_t initData5[] = {0xf0, 0xa5, 0x1b, 0x01, 0x02, 0xb3};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
    } else if (bike_type == TYPE::FYTTER_RI08) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x00, 0x00, 0x90};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x00, 0xc8, 0x58};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (bike_type == TYPE::CASALL) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x00, 0x00, 0x90};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x3b, 0x01, 0xcc};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x3b, 0x01, 0xcd};
        const uint8_t initData4[] = {0xf0, 0xa5, 0x3b, 0x01, 0x03, 0xd4};
        const uint8_t initData5[] = {0xf0, 0xa2, 0x3b, 0x01, 0xce};
        const uint8_t initData6[] = {0xf0, 0xa3, 0x3b, 0x01, 0x01, 0xd0};
        const uint8_t initData7[] = {0xf0, 0xa4, 0x3b, 0x01, 0x01, 0x01, 0x01, 0x01,
                                     0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xda};
        const uint8_t initData8[] = {0xf0, 0xa5, 0x3b, 0x01, 0x02, 0xd3};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        QThread::msleep(400);
        writeCharacteristic((uint8_t *)initData8, sizeof(initData8), QStringLiteral("init"), false, true);
        QThread::msleep(400);
    } else if (bike_type == TYPE::TAURUA_IC90) {
        const uint8_t initData1[] = {0xf0, 0xa0, 0x01, 0x00, 0x91};
        const uint8_t initData2[] = {0xf0, 0xa0, 0x01, 0x31, 0xc2};
        const uint8_t initData3[] = {0xf0, 0xa1, 0x01, 0x31, 0xc3};
        const uint8_t initData4[] = {0xf0, 0xa0, 0x01, 0x31, 0xc2};
        const uint8_t initData5[] = {0xf0, 0xa1, 0x01, 0x31, 0xc3};
        const uint8_t initData6[] = {0xf0, 0xa3, 0x01, 0x31, 0x01, 0xc6};
        const uint8_t initData7[] = {0xf0, 0xa4, 0x01, 0x31, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0xd0};
        const uint8_t initData8[] = {0xf0, 0xa5, 0x01, 0x31, 0x02, 0xc9};

        writeCharacteristic((uint8_t *)initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData7, sizeof(initData7), QStringLiteral("init"), false, true);
        writeCharacteristic((uint8_t *)initData8, sizeof(initData8), QStringLiteral("init"), false, true);
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
            bike_type == TYPE::JLL_IC400 || bike_type == TYPE::DKN_MOTION_2 || bike_type == TYPE::FYTTER_RI08 ||
            bike_type == TYPE::HERTZ_XR_770_2 || bike_type == TYPE::VIRTUFIT_2 || bike_type == TYPE::TUNTURI ||
            bike_type == TYPE::FITHIWAY || bike_type == TYPE::ENERFIT_SPX_9500_2 || bike_type == TYPE::REEBOK_2 ||
            bike_type == TYPE::FAL_SPORTS || bike_type == TYPE::TRXAPPGATE_TC || bike_type == HAMMER_SPEED_BIKE_S ||
            bike_type == TYPE::HOP_SPORT_HS_090H_2) {
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
            if (!gattNotify2Characteristic.isValid()) {
                if(bike_type == TYPE::IRUNNING_2)
                    bike_type = TYPE::ICONSOLE_2;
                else
                    bike_type = TYPE::ICONSOLE;
                qDebug() << QStringLiteral("ICONSOLE bike found - overrode due to characteristics");
            }
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
        if (!firstVirtualBike && !this->hasVirtualDevice()) {

            QSettings settings;
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));

                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&trxappgateusbbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &trxappgateusbbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
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
    QString uuid3 = QStringLiteral("0000fff0-0000-1000-8000-00805f9b34fb");
    if (bike_type == TYPE::IRUNNING || bike_type == TYPE::CHANGYOW || bike_type == TYPE::ICONSOLE ||
        bike_type == TYPE::JLL_IC400 || bike_type == TYPE::FYTTER_RI08 || bike_type == TYPE::TUNTURI ||
        bike_type == TYPE::FITHIWAY || bike_type == TYPE::FAL_SPORTS || bike_type == TYPE::TRXAPPGATE_TC ||
        bike_type == HAMMER_SPEED_BIKE_S) {
        uuid = uuid2;
    }

    if (bike_type == TYPE::HERTZ_XR_770) {
        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = HERTZ_XR_770_2;
            uuid = uuid2;
        }
    } else if (bike_type == ENERFIT_SPX_9500) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = ENERFIT_SPX_9500_2;
            uuid = uuid2;
        }
    } else if (bike_type == REEBOK) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = REEBOK_2;
            uuid = uuid2;
        }
    } else if (bike_type == DKN_MOTION) {

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
    } else if (bike_type == VIRTUFIT) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = VIRTUFIT_2;
            uuid = uuid2;
        }

    } else if (bike_type == ICONSOLE) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = ICONSOLE_2;
            uuid = uuid3;
        }
    } else if (bike_type == TUNTURI) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = TUNTURI_2;
            uuid = uuid3;
        }
    } else if (bike_type == IRUNNING) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = IRUNNING_2;
            uuid = uuid3;
        }
    } else if (bike_type == HOP_SPORT_HS_090H) {

        bool found = false;
        foreach (QBluetoothUuid s, m_control->services()) {

            if (s == QBluetoothUuid::fromString(uuid)) {
                found = true;
                break;
            }
        }
        if (!found) {
            bike_type = HOP_SPORT_HS_090H_2;
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
    bool JLL_IC400_bike = settings.value(QZSettings::jll_IC400_bike, QZSettings::default_jll_IC400_bike).toBool();
    bool FYTTER_ri08_bike = settings.value(QZSettings::fytter_ri08_bike, QZSettings::default_fytter_ri08_bike).toBool();
    bool ASVIVA_bike = settings.value(QZSettings::asviva_bike, QZSettings::default_asviva_bike).toBool();
    bool hertz_xr_770 = settings.value(QZSettings::hertz_xr_770, QZSettings::default_hertz_xr_770).toBool();
    bool enerfit_SPX_9500 = settings.value(QZSettings::enerfit_SPX_9500, QZSettings::default_enerfit_SPX_9500).toBool();
    bool hop_sport_hs_090h_bike = settings.value(QZSettings::hop_sport_hs_090h_bike, QZSettings::default_hop_sport_hs_090h_bike).toBool();
    bool toorx_bike_srx_500 = settings.value(QZSettings::toorx_bike_srx_500, QZSettings::default_toorx_bike_srx_500).toBool();
    bool taurua_ic90 = settings.value(QZSettings::taurua_ic90, QZSettings::default_taurua_ic90).toBool();
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    const QString deviceName = device.name();
    const QString upperDeviceName = deviceName.toUpper();
    bool isTrxAppGateTcName = false;
    if (upperDeviceName.startsWith(QStringLiteral("TC")) && deviceName.length() == 5) {
        isTrxAppGateTcName = true;
        for (int idx = 2; idx < deviceName.length(); ++idx) {
            if (!deviceName.at(idx).isDigit()) {
                isTrxAppGateTcName = false;
                break;
            }
        }
    }
    // if(device.name().startsWith("TOORX") || device.name().startsWith("V-RUN") || device.name().startsWith("FS-")
    // || device.name().startsWith("i-Console+") || device.name().startsWith("i-Running"))
    {
        // if(device.name().startsWith("i-Running") || device.name().startsWith("i-Console+"))
        bike_type = TYPE::IRUNNING;
        /*else
            bike_type = TYPE::TRXAPPGATE;*/

        if (hertz_xr_770) {
            refresh->start(500ms);

            bike_type = TYPE::HERTZ_XR_770;
            qDebug() << QStringLiteral("HERTZ_XR_770 bike found");
        } else if (enerfit_SPX_9500) {
            refresh->start(500ms);
            if (upperDeviceName.startsWith(QStringLiteral("IBIKING+"))) {
                bike_type = TYPE::HAMMER_SPEED_BIKE_S;
                qDebug() << QStringLiteral("HAMMER_SPEED_BIKE_S bike found");
            } else {
                bike_type = TYPE::ENERFIT_SPX_9500;
                qDebug() << QStringLiteral("ENERFIT_SPX_9500 bike found");
            }
        } else if (JLL_IC400_bike) {
            refresh->start(500ms);

            bike_type = TYPE::JLL_IC400;
            qDebug() << QStringLiteral("JLL_IC400 bike found");
        } else if (FYTTER_ri08_bike) {
            refresh->start(500ms);

            bike_type = TYPE::FYTTER_RI08;
            qDebug() << QStringLiteral("FYTTER_RI08 bike found");
        } else if (ASVIVA_bike) {
            refresh->start(500ms);

            bike_type = TYPE::ASVIVA;
            qDebug() << QStringLiteral("ASVIVA bike found");
        } else if(hop_sport_hs_090h_bike) {
            refresh->start(500ms);

            bike_type = TYPE::HOP_SPORT_HS_090H;
            qDebug() << QStringLiteral("hop_sport_hs_090h_bike bike found");
        } else if(toorx_bike_srx_500) {
            refresh->start(500ms);

            bike_type = TYPE::TOORX_SRX_500;
            qDebug() << QStringLiteral("TOORX_SRX_500 bike found");
        } else if(taurua_ic90) {
            refresh->start(500ms);

            bike_type = TYPE::TAURUA_IC90;
            qDebug() << QStringLiteral("TAURUA_IC90 bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("REEBOK"))) {
            bike_type = TYPE::REEBOK;
            qDebug() << QStringLiteral("REEBOK bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("TUN "))) {
            bike_type = TYPE::TUNTURI;
            qDebug() << QStringLiteral("TUNTURI bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("VIFHTR")) ||
                   upperDeviceName.startsWith(QStringLiteral("VIRTUFIT"))) {

            bike_type = TYPE::VIRTUFIT;
            qDebug() << QStringLiteral("VIRTUFIT bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("FITHIWAY")) ||
                   upperDeviceName.startsWith(QStringLiteral("FIT HI WAY"))) {
            bike_type = TYPE::FITHIWAY;
            qDebug() << QStringLiteral("FITHIWAY bike found");           
        } else if (device.address().toString().toUpper().startsWith(QStringLiteral("E8"))) {

            bike_type = TYPE::CHANGYOW;
            qDebug() << QStringLiteral("CHANGYOW bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("BFCP"))) {

            bike_type = TYPE::SKANDIKAWIRY;
            qDebug() << QStringLiteral("SKANDIKAWIRY bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("I-CONSOIE+"))) {

            bike_type = TYPE::CASALL;
            qDebug() << QStringLiteral("CASALL bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("ICONSOLE+"))) {

            bike_type = TYPE::ICONSOLE;
            qDebug() << QStringLiteral("ICONSOLE bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("DKN MOTION"))) {

            bike_type = TYPE::DKN_MOTION;
            qDebug() << QStringLiteral("DKN MOTION bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("BIKZU_"))) {

            bike_type = TYPE::BIKZU;
            qDebug() << QStringLiteral("BIKZU bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("PASYOU-"))) {

            bike_type = TYPE::PASYOU;
            qDebug() << QStringLiteral("PASYOU bike found");
        } else if (isTrxAppGateTcName) {

            bike_type = TYPE::TRXAPPGATE_TC;
            qDebug() << QStringLiteral("TRXAPPGATE TC bike found");
        } else if (upperDeviceName.startsWith(QStringLiteral("FAL-SPORTS"))) {

            bike_type = TYPE::FAL_SPORTS;
            qDebug() << QStringLiteral("FAL-SPORTS bike found");
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

void trxappgateusbbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

uint16_t trxappgateusbbike::wattsFromResistance(double resistance) {
    QSettings settings;
    bool toorx_srx_3500 = settings.value(QZSettings::toorx_srx_3500, QZSettings::default_toorx_srx_3500).toBool();
    if(toorx_srx_3500) {
        double P;
        // Toorx SRX 3500 #1999
        P = 37.069 
            - 1.483 * Cadence.value() 
            - 4.942 * resistance 
            + 0.023 * Cadence.value() * Cadence.value() 
            + 0.336 * Cadence.value() * resistance 
            - 0.036 * resistance * resistance;
        return P;
    } else {
        return _ergTable.estimateWattage(Cadence.value(), resistance);
    }
}

resistance_t trxappgateusbbike::resistanceFromPowerRequest(uint16_t power) {
    return _ergTable.resistanceFromPowerRequest(power, Cadence.value(), maxResistance());
}

void trxappgateusbbike::resistanceFromFTMSAccessory(resistance_t res) {
    ResistanceFromFTMSAccessory = res;
    ResistanceFromFTMSAccessoryLastTime = QDateTime::currentMSecsSinceEpoch();
    qDebug() << QStringLiteral("resistanceFromFTMSAccessory") << res;
}
