#include "proformellipticaltrainer.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>
#include <qmath.h>

using namespace std::chrono_literals;

proformellipticaltrainer::proformellipticaltrainer(bool noWriteResistance, bool noHeartService,
                                                   int8_t bikeResistanceOffset, double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &proformellipticaltrainer::update);
    refresh->start(200ms);
}

void proformellipticaltrainer::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
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

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void proformellipticaltrainer::forceResistance(resistance_t requestResistance) {

    const uint8_t res[] = {0xfe, 0x02, 0x0d, 0x02};
    writeCharacteristic((uint8_t *)res, sizeof(res), QStringLiteral("resistance"), false, false);

    const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0x76, 0x01, 0x00, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0x17, 0x03, 0x00, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0xb8, 0x04, 0x00, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0x58, 0x06, 0x00, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0xf9, 0x07, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0x9a, 0x09, 0x00, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0x3a, 0x0b, 0x00, 0x5e, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0xdb, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                            0x04, 0x7c, 0x0e, 0x00, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x1c, 0x10, 0x00, 0x45, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0xbd, 0x11, 0x00, 0xe7, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x5e, 0x13, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0xfe, 0x14, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x9f, 0x16, 0x00, 0xce, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x40, 0x18, 0x00, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0xe0, 0x19, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x81, 0x1b, 0x00, 0xb5, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x22, 0x1d, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0xc2, 0x1e, 0x00, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x63, 0x20, 0x00, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res21[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x04, 0x22, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res22[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0xa4, 0x23, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res23[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0x45, 0x25, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8_t res24[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x09, 0x09, 0x02, 0x01,
                             0x04, 0xe6, 0x26, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};

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
    case 21:
        writeCharacteristic((uint8_t *)res21, sizeof(res21), QStringLiteral("resistance21"), false, true);
        break;
    case 22:
        writeCharacteristic((uint8_t *)res22, sizeof(res22), QStringLiteral("resistance22"), false, true);
        break;
    case 23:
        writeCharacteristic((uint8_t *)res23, sizeof(res23), QStringLiteral("resistance23"), false, true);
        break;
    case 24:
        writeCharacteristic((uint8_t *)res24, sizeof(res24), QStringLiteral("resistance24"), false, true);
        break;
    }
}

void proformellipticaltrainer::update() {

    if (!m_control)
        return;

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
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x09, 0x13, 0x02, 0x00,
                                   0x0d, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0x7b, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x09, 0x10, 0x02, 0x00,
                                   0x0a, 0x3e, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x02, 0x18, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
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
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
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

void proformellipticaltrainer::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void proformellipticaltrainer::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double proformellipticaltrainer::GetResistanceFromPacket(QByteArray packet) {
    uint8_t r = (uint8_t)(packet.at(13));
    switch (r) {
    case 1:
        return 1;
    case 3:
        return 2;
    case 4:
        return 3;
    case 6:
        return 4;
    case 7:
    case 8:
        return 5;
    case 9:
        return 6;
    case 0xb:
        return 7;
    case 0xc:
    case 0xd:
        return 8;
    case 0xe:
        return 9;
    case 0x10:
        return 10;
    case 0x11:
        return 11;
    case 0x13:
        return 12;
    case 0x14:
    case 0x15:
        return 13;
    case 0x16:
        return 14;
    case 0x18:
        return 15;
    case 0x19:
    case 0x1a:
        return 16;
    case 0x1b:
        return 17;
    case 0x1d:
        return 18;
    case 0x1e:
        return 19;
    case 0x20:
        return 20;
    case 0x22:
        return 21;
    case 0x23:
        return 22;
    case 0x25:
        return 23;
    case 0x26:
    case 0x27:
        return 24;
    }
    return 1;
}

void proformellipticaltrainer::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                                     const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
    double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();
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

    /*
    if (newValue.length() == 20 && (uint8_t)newValue.at(0) == 0xff && newValue.at(1) == 0x10 &&
        newValue.at(2) == 0x01) {
        Speed = (double)(((uint16_t)((uint8_t)newValue.at(4)) << 8) + (uint16_t)((uint8_t)newValue.at(3))) / 100.0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        return;
    }*/

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
        newValue.at(3) != 0x04 || newValue.at(4) != 0x02 || (newValue.at(5) != 0x2e && newValue.at(5) != 0x30) ||
        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) {
        return;
    }

    Speed = ((double)((uint8_t)newValue.at(14)) / 10.0) * miles;
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
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
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
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

void proformellipticaltrainer::btinit() {

    {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x09, 0x04, 0x80, 0x8d,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x09, 0x04, 0x88, 0x95,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x09, 0x28, 0x90, 0x04,
                                0x00, 0x34, 0xfc, 0xbe, 0x86, 0x40, 0x18, 0xd2, 0xa2, 0x7c};
        uint8_t initData11[] = {0x01, 0x12, 0x44, 0x16, 0xfe, 0xc8, 0xb0, 0x6a, 0x4a, 0x24,
                                0x0c, 0xee, 0xf6, 0xd0, 0xa8, 0x82, 0x92, 0x6c, 0x74, 0x46};
        uint8_t initData12[] = {0xff, 0x08, 0x4e, 0x58, 0x60, 0x80, 0x02, 0x00, 0x00, 0x53,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x09, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x6b, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x09, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3f, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x09, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xab, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0xfe, 0x02, 0x14, 0x03};
        uint8_t noOpData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x09, 0x10, 0x02, 0x00,
                                0x0a, 0x3e, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData11[] = {0xff, 0x02, 0x18, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
        QThread::msleep(400);
    }

    initDone = true;
}

void proformellipticaltrainer::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("00001534-1412-efde-1523-785feabcd123"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("00001535-1412-efde-1523-785feabcd123"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &proformellipticaltrainer::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &proformellipticaltrainer::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &proformellipticaltrainer::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &proformellipticaltrainer::descriptorWritten);

        // ******************************************* virtual treadmill init *************************************
        QSettings settings;
        if (!firstStateChanged && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &proformellipticaltrainer::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &proformellipticaltrainer::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset,
                                                       bikeResistanceGain);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &proformellipticaltrainer::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstStateChanged = 1;
            }
        }
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void proformellipticaltrainer::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void proformellipticaltrainer::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                                     const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void proformellipticaltrainer::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this,
            &proformellipticaltrainer::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void proformellipticaltrainer::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("proformellipticaltrainer::errorService") +
               QString::fromLocal8Bit(metaEnum.valueToKey(err)) + m_control->errorString());
}

void proformellipticaltrainer::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("proformellipticaltrainer::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformellipticaltrainer::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_TL")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this,
                &proformellipticaltrainer::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &proformellipticaltrainer::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &proformellipticaltrainer::error);
        connect(m_control, &QLowEnergyController::stateChanged, this,
                &proformellipticaltrainer::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
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

bool proformellipticaltrainer::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void proformellipticaltrainer::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

int proformellipticaltrainer::pelotonToEllipticalResistance(int pelotonResistance) {
    return ceil((pelotonResistance - 10.0) / 3.33);
}
