#include "proformbike.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

proformbike::proformbike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &proformbike::update);
    refresh->start(200ms);
}

void proformbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                      bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
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
}

void proformbike::forceResistance(int8_t requestResistance) {
    const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0x32, 0x02, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0xa3, 0x04, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0x14, 0x07, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0x85, 0x09, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0xf6, 0x0b, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0x67, 0x0e, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0xd8, 0x10, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0x49, 0x13, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                            0x04, 0xba, 0x15, 0x00, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                             0x04, 0x2b, 0x18, 0x00, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                             0x04, 0x9c, 0x1a, 0x00, 0xcd, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                             0x04, 0x0d, 0x1d, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                             0x04, 0x7e, 0x1f, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                             0x04, 0xef, 0x21, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                             0x04, 0x60, 0x24, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                             0x04, 0xd1, 0x26, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00};

    switch (requestResistance) {
    case 1:
        writeCharacteristic((uint8_t *)res1, sizeof(res1), QStringLiteral("resistance1"), false, true);
        break;
    case 2:
        writeCharacteristic((uint8_t *)res2, sizeof(res2), QStringLiteral("resistance2"), false, true);
        break;
    case 3:
        writeCharacteristic((uint8_t *)res3, sizeof(res3), QStringLiteral("resistance3"), false, true);
        break;
    case 4:
        writeCharacteristic((uint8_t *)res4, sizeof(res4), QStringLiteral("resistance4"), false, true);
        break;
    case 5:
        writeCharacteristic((uint8_t *)res5, sizeof(res5), QStringLiteral("resistance5"), false, true);
        break;
    case 6:
        writeCharacteristic((uint8_t *)res6, sizeof(res6), QStringLiteral("resistance6"), false, true);
        break;
    case 7:
        writeCharacteristic((uint8_t *)res7, sizeof(res7), QStringLiteral("resistance7"), false, true);
        break;
    case 8:
        writeCharacteristic((uint8_t *)res8, sizeof(res8), QStringLiteral("resistance8"), false, true);
        break;
    case 9:
        writeCharacteristic((uint8_t *)res9, sizeof(res9), QStringLiteral("resistance9"), false, true);
        break;
    case 10:
        writeCharacteristic((uint8_t *)res10, sizeof(res10), QStringLiteral("resistance10"), false, true);
        break;
    case 11:
        writeCharacteristic((uint8_t *)res11, sizeof(res11), QStringLiteral("resistance11"), false, true);
        break;
    case 12:
        writeCharacteristic((uint8_t *)res12, sizeof(res12), QStringLiteral("resistance12"), false, true);
        break;
    case 13:
        writeCharacteristic((uint8_t *)res13, sizeof(res13), QStringLiteral("resistance13"), false, true);
        break;
    case 14:
        writeCharacteristic((uint8_t *)res14, sizeof(res14), QStringLiteral("resistance14"), false, true);
        break;
    case 15:
        writeCharacteristic((uint8_t *)res15, sizeof(res15), QStringLiteral("resistance15"), false, true);
        break;
    case 16:
        writeCharacteristic((uint8_t *)res16, sizeof(res16), QStringLiteral("resistance16"), false, true);
        break;
    }
}

void proformbike::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        update_metrics(true, watts());

        QSettings settings;
        bool proform_tour_de_france_clc = settings.value(QStringLiteral("proform_tour_de_france_clc"), false).toBool();

        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x00,
                               0x0f, 0xbc, 0x90, 0x70, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x5d, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00,
                               0x0d, 0x3c, 0x9c, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa9, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};

        // proform_tour_de_france_clc
        uint8_t noOpData2_proform_tour_de_france_clc[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x00,
                                                          0x0f, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3_proform_tour_de_france_clc[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xfd, 0x00,
                                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6_proform_tour_de_france_clc[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0xb3, 0x00, 0x00, 0x00,
                                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        switch (counterPoll) {
        case 0:
            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
            break;
        case 1:
            if (proform_tour_de_france_clc)
                writeCharacteristic(noOpData2_proform_tour_de_france_clc, sizeof(noOpData2_proform_tour_de_france_clc),
                                    QStringLiteral("noOp"));
            else
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
            break;
        case 2:
            if (proform_tour_de_france_clc)
                writeCharacteristic(noOpData3_proform_tour_de_france_clc, sizeof(noOpData3_proform_tour_de_france_clc),
                                    QStringLiteral("noOp"));
            else
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"));
            break;
        case 3:
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
            break;
        case 4:
            writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
            break;
        case 5:
            if (proform_tour_de_france_clc)
                writeCharacteristic(noOpData6_proform_tour_de_france_clc, sizeof(noOpData6_proform_tour_de_france_clc),
                                    QStringLiteral("noOp"));
            else
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
            break;
        case 6:
            writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("noOp"));
            if (requestResistance != -1) {
                if (requestResistance > 16) {
                    requestResistance = 16;
                } else if (requestResistance == 0) {
                    requestResistance = 1;
                }

                if (requestResistance != currentResistance().value()) {
                    emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                    forceResistance(requestResistance);
                }
                requestResistance = -1;
            }
            break;
        }

        counterPoll++;
        if (counterPoll > 6) {
            counterPoll = 0;
        } else if (counterPoll == 6 && proform_tour_de_france_clc && requestResistance == -1) {
            // this bike sends the frame noOpData7 only when it needs to change the resistance
            counterPoll = 0;
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
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

int proformbike::pelotonToBikeResistance(int pelotonResistance) {
    if (pelotonResistance <= 10) {
        return 1;
    }
    if (pelotonResistance <= 20) {
        return 2;
    }
    if (pelotonResistance <= 25) {
        return 3;
    }
    if (pelotonResistance <= 30) {
        return 4;
    }
    if (pelotonResistance <= 35) {
        return 5;
    }
    if (pelotonResistance <= 40) {
        return 6;
    }
    if (pelotonResistance <= 45) {
        return 7;
    }
    if (pelotonResistance <= 50) {
        return 8;
    }
    if (pelotonResistance <= 55) {
        return 9;
    }
    if (pelotonResistance <= 60) {
        return 10;
    }
    if (pelotonResistance <= 65) {
        return 11;
    }
    if (pelotonResistance <= 70) {
        return 12;
    }
    if (pelotonResistance <= 75) {
        return 13;
    }
    if (pelotonResistance <= 80) {
        return 14;
    }
    if (pelotonResistance <= 85) {
        return 15;
    }
    if (pelotonResistance <= 100) {
        return 16;
    }
    return Resistance.value();
}

void proformbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void proformbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
        newValue.at(3) != 0x04 ||
        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) {
        return;
    }

    m_watts = ((uint16_t)(((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12)));

    // filter some strange values from proform
    if (m_watts > 3000) {
        m_watts = 0;
    } else {
        switch ((uint8_t)newValue.at(11)) {
        case 0x02:
            Resistance = 1;
            m_pelotonResistance = 10;
            break;
        case 0x04:
            Resistance = 2;
            m_pelotonResistance = 20;
            break;
        case 0x07:
            Resistance = 3;
            m_pelotonResistance = 25;
            break;
        case 0x09:
            Resistance = 4;
            m_pelotonResistance = 30;
            break;
        case 0x0b:
            Resistance = 5;
            m_pelotonResistance = 35;
            break;
        case 0x0e:
            Resistance = 6;
            m_pelotonResistance = 40;
            break;
        case 0x10:
            Resistance = 7;
            m_pelotonResistance = 45;
            break;
        case 0x13:
            Resistance = 8;
            m_pelotonResistance = 50;
            break;
        case 0x15:
            Resistance = 9;
            m_pelotonResistance = 55;
            break;
        case 0x18:
            Resistance = 10;
            m_pelotonResistance = 60;
            break;
        case 0x1a:
            Resistance = 11;
            m_pelotonResistance = 65;
            break;
        case 0x1d:
            Resistance = 12;
            m_pelotonResistance = 70;
            break;
        case 0x1f:
            Resistance = 13;
            m_pelotonResistance = 75;
            break;
        case 0x21:
            Resistance = 14;
            m_pelotonResistance = 80;
            break;
        case 0x24:
            Resistance = 15;
            m_pelotonResistance = 85;
            break;
        case 0x26:
            Resistance = 16;
            m_pelotonResistance = 100;
            break;
            /* when the proform bike is changing the resistance, it sends some strange values, so i'm keeping the last
            good one default: Resistance = 0; m_pelotonResistance = 0; break;
            */
        }
        emit resistanceRead(Resistance.value());

        if (settings.value(QStringLiteral("cadence_sensor_name"), QStringLiteral("Disabled"))
                .toString()
                .startsWith(QStringLiteral("Disabled"))) {
            Cadence = ((uint8_t)newValue.at(18));
        }

        if (!settings.value(QStringLiteral("speed_power_based"), false).toBool()) {
            Speed =
                (settings.value(QStringLiteral("proform_wheel_ratio"), 0.33).toDouble()) * ((double)Cadence.value());
        } else {
            Speed = metric::calculateSpeedFromPower(m_watt.value());
        }
        if (watts())
            KCal +=
                ((((0.048 * ((double)watts()) + 1.19) * settings.value(QStringLiteral("weight"), 75.0).toFloat() *
                   3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }

        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

#ifdef Q_OS_ANDROID
        if (settings.value("ant_heart", false).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
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
            }
        }

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence = settings.value("bike_cadence_sensor", false).toBool();
        bool ios_peloton_workaround = settings.value("ios_peloton_workaround", false).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif
#endif

        emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        // debug("Current Distance: " + QString::number(distance));
        emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
        emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

        if (m_control->error() != QLowEnergyController::NoError) {
            qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
        }
    }
}

void proformbike::btinit() {

    QSettings settings;

    uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
    uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x80, 0x8b,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x88, 0x93,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
    uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                           0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                           0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};

    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
    QThread::msleep(400);
    writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
    QThread::msleep(400);

    if (settings.value(QStringLiteral("proform_tour_de_france_clc"), false).toBool()) {

        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07,
                                0x01, 0xb9, 0xf8, 0x45, 0x80, 0xc9, 0x10, 0x6d, 0xb8, 0x09};
        uint8_t initData11[] = {0x01, 0x12, 0x58, 0xa5, 0xf0, 0x59, 0xa0, 0x1d, 0x78, 0xd9,
                                0x38, 0x85, 0xe0, 0x49, 0xd0, 0x2d, 0xb8, 0x09, 0x98, 0xe5};
        uint8_t initData12[] = {0xff, 0x08, 0x70, 0xf9, 0x40, 0x98, 0x02, 0x00, 0x00, 0xfc,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(400);

    } else {

        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07,
                                0x01, 0x10, 0xcc, 0x7a, 0x3e, 0xf4, 0xb8, 0x66, 0x3a, 0xf8};
        uint8_t initData11[] = {0x01, 0x12, 0xb4, 0x72, 0x46, 0x1c, 0xf0, 0xbe, 0x92, 0x40,
                                0x3c, 0xea, 0xce, 0xa4, 0x88, 0x76, 0x4a, 0x28, 0x04, 0xe2};
        uint8_t initData12[] = {0xff, 0x08, 0xf6, 0xcc, 0xe0, 0x98, 0x02, 0x00, 0x00, 0xd1,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(400);
    }

    initDone = true;
}

void proformbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("00001534-1412-efde-1523-785feabcd123"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("00001535-1412-efde-1523-785feabcd123"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &proformbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &proformbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &proformbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &proformbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
        if (!firstStateChanged && !virtualBike
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            && !h
#endif
#endif
        ) {
            QSettings settings;
            bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence = settings.value("bike_cadence_sensor", false).toBool();
            bool ios_peloton_workaround = settings.value("ios_peloton_workaround", false).toBool();
            if (ios_peloton_workaround && cadence) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&proformbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &proformbike::inclinationChanged);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void proformbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void proformbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void proformbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &proformbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void proformbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("proformbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("proformbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_EB")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &proformbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &proformbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &proformbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &proformbike::controllerStateChanged);

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

bool proformbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *proformbike::VirtualBike() { return virtualBike; }

void *proformbike::VirtualDevice() { return VirtualBike(); }

uint16_t proformbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watts;
}

void proformbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
