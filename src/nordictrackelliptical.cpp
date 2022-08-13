#include "nordictrackelliptical.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

nordictrackelliptical::nordictrackelliptical(bool noWriteResistance, bool noHeartService,
                                                   uint8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &nordictrackelliptical::update);
    refresh->start(200ms);
}

void nordictrackelliptical::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
                                                   bool disable_log, bool wait_for_response) {
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

void nordictrackelliptical::forceResistance(resistance_t requestResistance) {

    const uint8_t res[] = {0xfe, 0x02, 0x0d, 0x02};
    writeCharacteristic((uint8_t *)res, sizeof(res), QStringLiteral("resistance"), false, false);

    const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0xc2, 0x01, 0x00, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0xb6, 0x03, 0x00, 0xcf, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0xaa, 0x05, 0x00, 0xc5, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x9e, 0x07, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x92, 0x09, 0x00, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x86, 0x0b, 0x00, 0xa7, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x7a, 0x0d, 0x00, 0x9d, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x6e, 0x0f, 0x00, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x62, 0x11, 0x00, 0x89, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x56, 0x13, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x4a, 0x15, 0x00, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x3e, 0x17, 0x00, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x32, 0x19, 0x00, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x26, 0x1b, 0x00, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x1a, 0x1d, 0x00, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x0e, 0x1f, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0x02, 0x21, 0x00, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0xf6, 0x22, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0xea, 0x24, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01, 0x04, 0xde, 0x26, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00};

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
    case 17:
        writeCharacteristic((uint8_t *)res17, sizeof(res17), QStringLiteral("resistance17"), false, true);
        break;
    case 18:
        writeCharacteristic((uint8_t *)res18, sizeof(res18), QStringLiteral("resistance18"), false, true);
        break;
    case 19:
        writeCharacteristic((uint8_t *)res19, sizeof(res19), QStringLiteral("resistance19"), false, true);
        break;
    case 20:
        writeCharacteristic((uint8_t *)res20, sizeof(res20), QStringLiteral("resistance20"), false, true);
        break;
    }
}

void nordictrackelliptical::update() {
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
        QSettings settings;
        update_metrics(true, watts());

        {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x0d, 0x02};
            uint8_t noOpData5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x00, 0x03, 0x80, 0x00, 0x40, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"));
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                if (requestResistance != -1) {
                    if (requestResistance < 0)
                        requestResistance = 0;
                    if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                        requestResistance <= max_resistance) {
                        emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                        forceResistance(requestResistance);
                    }
                    requestResistance = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 4) {
                counterPoll = 0;
            }
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }
        /*
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

        // btinit();

        requestStart = -1;
        emit tapeStarted();
    }*/
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void nordictrackelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void nordictrackelliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double nordictrackelliptical::GetResistanceFromPacket(QByteArray packet) {
    uint8_t r = (uint8_t)(packet.at(11));
    switch (r) {
    case 1:
        return 1;
    case 3:
        return 2;
    case 5:
        return 3;
    case 7:
    case 8:
        return 4;
    case 9:
        return 5;
    case 0xb:
        return 6;
    case 0xc:
    case 0xd:
        return 7;
    case 0xf:
        return 8;
    case 0x11:
        return 9;
    case 0x13:
        return 10;
    case 0x14:
    case 0x15:
        return 11;
    case 0x17:
        return 12;
    case 0x19:
    case 0x1a:
        return 13;
    case 0x1b:
        return 14;
    case 0x1d:
        return 15;
    case 0x1f:
        return 16;
    case 0x21:
        return 17;
    case 0x22:
    case 0x23:
        return 18;
    case 0x24:
    case 0x25:
        return 19;
    case 0x26:
    case 0x27:
        return 20;
    }
    return 1;
}

void nordictrackelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                     const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();
    double weight = settings.value(QStringLiteral("weight"), 75.0).toFloat();
    double cadence_gain = settings.value(QStringLiteral("cadence_gain"), 1.0).toDouble();
    double cadence_offset = settings.value(QStringLiteral("cadence_offset"), 0.0).toDouble();
    const double miles = 1.60934;

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() == 20 && newValue.at(0) == 0x01 && newValue.at(1) == 0x12 && newValue.at(19) == 0x2C) {
        uint8_t c = newValue.at(2);
        if (c > 0)
            Cadence = (c * cadence_gain) + cadence_offset;
        else
            Cadence = 0;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        if (Cadence.value() > 0) {
            CrankRevs++;
            LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
        }
        return;
    }

    if (newValue.length() == 20 && newValue.at(0) == 0x01 && newValue.at(1) == 0x12 && newValue.at(2) == 0x46) {
        Speed = (double)(((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t)newValue.at(14))) / 100.0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        lastSpeedChanged = QDateTime::currentDateTime();
    } else if(QDateTime::currentDateTime().secsTo(lastSpeedChanged) > 3) {
        Speed = 0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    }

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
        newValue.at(3) != 0x04 || newValue.at(4) != 0x02 || (newValue.at(5) != 0x2e && newValue.at(5) != 0x30) ||
        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) {
        return;
    }

    //wattage = newValue.at(12)

    Resistance = GetResistanceFromPacket(newValue);

    uint16_t p = qCeil((Resistance.value() * 3.33) + 10.0);
    if (p > 100)
        p = 100;
    m_pelotonResistance = p;

    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));

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

    emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void nordictrackelliptical::btinit() {

    {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x80, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x88, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x06, 0x28, 0x90, 0x04, 0x00, 0x2e, 0x44, 0x50, 0x6a, 0x82, 0xa8, 0xb4, 0xd6, 0xf6};
        uint8_t initData11[] = {0x01, 0x12, 0x2c, 0x48, 0x62, 0x9a, 0xd0, 0xfc, 0x0e, 0x5e, 0x94, 0xa0, 0xfa, 0x32, 0x78, 0x84, 0xc6, 0x06, 0x7c, 0xb8};
        uint8_t initData12[] = {0xff, 0x08, 0xf2, 0x2a, 0xa0, 0x80, 0x02, 0x00, 0x00, 0xda, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x06, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xc0, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData10[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(400);

        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);

        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(400);
    }

    initDone = true;
}

void nordictrackelliptical::stateChanged(QLowEnergyService::ServiceState state) {
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
                &nordictrackelliptical::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &nordictrackelliptical::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &nordictrackelliptical::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &nordictrackelliptical::descriptorWritten);

        // ******************************************* virtual treadmill init *************************************
        QSettings settings;
        if (!firstStateChanged && !virtualTreadmill && !virtualBike) {
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            bool virtual_device_force_bike = settings.value("virtual_device_force_bike", false).toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &nordictrackelliptical::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &nordictrackelliptical::changeInclinationRequested);
                } else {
                    debug("creating virtual bike interface...");
                    virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset,
                                                  bikeResistanceGain);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &nordictrackelliptical::changeInclinationRequested);
                }
                firstStateChanged = 1;
            }
        }
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void nordictrackelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void nordictrackelliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                     const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void nordictrackelliptical::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &nordictrackelliptical::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void nordictrackelliptical::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("nordictrackelliptical::errorService") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void nordictrackelliptical::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("nordictrackelliptical::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void nordictrackelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_TL")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this,
                &nordictrackelliptical::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &nordictrackelliptical::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &nordictrackelliptical::error);
        connect(m_control, &QLowEnergyController::stateChanged, this,
                &nordictrackelliptical::controllerStateChanged);

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

bool nordictrackelliptical::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *nordictrackelliptical::VirtualTreadmill() { return virtualTreadmill; }

void *nordictrackelliptical::VirtualDevice() { return VirtualTreadmill(); }

void nordictrackelliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

int nordictrackelliptical::pelotonToEllipticalResistance(int pelotonResistance) {
    return ceil((pelotonResistance - 10.0) / 3.33);
}
