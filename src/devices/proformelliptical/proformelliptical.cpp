#include "proformelliptical.h"
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

using namespace std::chrono_literals;

proformelliptical::proformelliptical(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;

    QSettings settings;
    nordictrack_se7i = settings.value(QZSettings::nordictrack_se7i, QZSettings::default_nordictrack_se7i).toBool();

    connect(refresh, &QTimer::timeout, this, &proformelliptical::update);
    refresh->start(200ms);
}

void proformelliptical::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

void proformelliptical::update() {
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

        if (nordictrack_se7i) {
            // NordicTrack Elliptical SE7i sendPoll cycle (6 packets)
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13,
                                   0x02, 0x00, 0x0d, 0x3e, 0x96, 0x33, 0x00, 0x10,
                                   0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x05, 0x54, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x06, 0x13,
                                   0x02, 0x00, 0x0d, 0x80, 0x00, 0x40, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x90, 0x78, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"));
                if (requestInclination != -100) {
                    if (requestInclination < 0)
                        requestInclination = 0;
                    if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                        requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestResistance != -1) {
                    if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                        requestResistance <= 24) {
                        emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                        forceResistance(requestResistance);
                    }
                    requestResistance = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));
                    requestStart = -1;
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x13, 0x13, 0x02, 0x00,
                                   0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0xc4, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x11, 0x02};
            uint8_t noOpData5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x13, 0x0d, 0x02, 0x00,
                                   0x07, 0xbc, 0x90, 0x70, 0x00, 0x00, 0x00, 0x40, 0x25, 0x00};

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
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), true);
                if (requestInclination != -100) {
                    if (requestInclination < 0)
                        requestInclination = 0;
                    if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                        requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        // forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                /*
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        // forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }*/
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
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

void proformelliptical::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void proformelliptical::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double proformelliptical::GetResistanceFromPacket(QByteArray packet) {
    uint8_t r = (uint8_t)(packet.at(11));
    switch (r) {
    case 1:
        return 1;
    case 3:
        return 2;
    case 5:
        return 3;
    case 7:
        return 4;
    case 8:
        return 5;
    case 10:
        return 6;
    case 12:
        return 7;
    case 14:
        return 8;
    case 15:
        return 9;
    case 17:
        return 10;
    case 19:
        return 11;
    case 21:
        return 12;
    case 23:
        return 13;
    case 24:
        return 14;
    case 26:
        return 15;
    case 28:
        return 16;
    case 30:
        return 17;
    case 31:
        return 18;
    case 35:
        return 20;
    case 39:
        return 22;
    }
    return 1;
}

void proformelliptical::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
    double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() == 20 && newValue.at(0) == 0x01 && newValue.at(1) == 0x12 && ((uint8_t)newValue.at(4)) != 0xFF) {
        double speed_from_machinery =
            (double)(((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t)newValue.at(14))) / 100.0;
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
            Speed = speed_from_machinery;
        } else {
            Speed = speedFromWatts();
        }
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        return;
    }

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
        newValue.at(3) != 0x04 || newValue.at(4) != 0x02 || (newValue.at(5) != 0x2e && newValue.at(5) != 0x30) ||
        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) {
        return;
    }

    if(newValue.at(11) == 0) {
        qDebug() << QStringLiteral("filtering resistance hole");
        return;
    }

    Resistance = GetResistanceFromPacket(newValue);
    Cadence = (newValue.at(18) * cadence_gain) + cadence_offset;
    m_watt = (double)(((uint16_t)((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12)));
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

void proformelliptical::btinit() {

    if(1)
    {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x13, 0x04, 0x80, 0x97,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x13, 0x04, 0x88, 0x9f,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x13, 0x28, 0x90, 0x04,
                                0x00, 0xbc, 0x5c, 0xf6, 0x96, 0x28, 0xd8, 0x7a, 0x12, 0xb4};
        uint8_t initData11[] = {0x01, 0x12, 0x64, 0x1e, 0xce, 0x60, 0x30, 0xc2, 0x9a, 0x2c,
                                0xec, 0xa6, 0x66, 0x38, 0xe8, 0xaa, 0x62, 0x24, 0x14, 0xce};
        uint8_t initData12[] = {0xff, 0x08, 0xbe, 0x70, 0x60, 0x70, 0x02, 0x00, 0x00, 0x45,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x13, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xb5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x13, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xc0, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x5d, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x13, 0x13, 0x02, 0x00,
                               0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0xc4, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x13, 0x13, 0x02, 0x0c,
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
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        /*writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);*/
    } else if (nordictrack_se7i) {
        // NordicTrack Elliptical SE7i initialization frames (19 packets: pkt944 to pkt1020)
        uint8_t initData1[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x80, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x06, 0x04, 0x88, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[4] = {0xfe, 0x02, 0x0b, 0x02};
        uint8_t initData8[20] = {0xff, 0x0b, 0x02, 0x04, 0x02, 0x07, 0x02, 0x07, 0x82, 0x00, 0x00, 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[4] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[20] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[4] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x06, 0x28, 0x90, 0x04, 0x00, 0x0d, 0x68, 0xc9, 0x28, 0x95, 0xf0, 0x69, 0xc0, 0x3d};
        uint8_t initData15[20] = {0x01, 0x12, 0xa8, 0x19, 0x88, 0xf5, 0x60, 0xf9, 0x70, 0xcd, 0x48, 0xc9, 0x48, 0xf5, 0x70, 0xe9, 0x60, 0x1d, 0x88, 0x39};
        uint8_t initData16[20] = {0xff, 0x08, 0xa8, 0x55, 0xc0, 0x80, 0x02, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData17[4] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData18[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x06, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData19[20] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        int sleepms = 400;

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    }

    initDone = true;
}

void proformelliptical::stateChanged(QLowEnergyService::ServiceState state) {
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
                &proformelliptical::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &proformelliptical::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &proformelliptical::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &proformelliptical::descriptorWritten);

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
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &proformelliptical::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &proformelliptical::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &proformelliptical::changeInclinationRequested);
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
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void proformelliptical::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void proformelliptical::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                              const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void proformelliptical::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &proformelliptical::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void proformelliptical::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("proformelliptical::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformelliptical::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("proformelliptical::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformelliptical::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_TL")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &proformelliptical::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &proformelliptical::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &proformelliptical::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &proformelliptical::controllerStateChanged);

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

bool proformelliptical::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void proformelliptical::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

void proformelliptical::forceIncline(double incline) {
    if (nordictrack_se7i) {
        // NordicTrack Elliptical SE7i incline control
        // Incline uses ff 0d packet with byte[10]=0x02
        // Formula: value = incline% * 100 (range 0-1000 for 0%-10%)
        uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
        uint8_t write[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                           0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        // Encode incline: bytes[11:12] = incline% * 100 (little-endian 16-bit)
        uint16_t incline_value = (uint16_t)(incline * 100);
        write[11] = incline_value & 0xFF;           // Low byte
        write[12] = (incline_value >> 8) & 0xFF;    // High byte

        // Calculate checksum in byte[14]: sum of bytes[0:14]
        uint8_t checksum = 0;
        for (int i = 0; i < 14; i++) {
            checksum += write[i];
        }
        write[14] = checksum;

        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("forceIncline"));
        writeCharacteristic(write, sizeof(write), QStringLiteral("forceIncline"), false, true);
    }
}

void proformelliptical::forceSpeed(double speed) {
    // Not used for ellipticals - kept for compatibility
    Q_UNUSED(speed);
}

void proformelliptical::forceResistance(double resistance) {
    if (nordictrack_se7i) {
        // NordicTrack Elliptical SE7i resistance control
        // Resistance uses ff 0d packet with byte[10]=0x04
        // Formula: value = resistance * 454 - 1 (range 453-9987 for resistance 1-21)
        uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
        uint8_t write[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x06, 0x09, 0x02, 0x01,
                           0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        // Encode resistance: bytes[11:12] = resistance * 454 - 1 (little-endian 16-bit)
        if (resistance >= 1 && resistance <= 21) {
            uint16_t resistance_value = (uint16_t)(resistance * 454 - 1);
            write[11] = resistance_value & 0xFF;           // Low byte
            write[12] = (resistance_value >> 8) & 0xFF;    // High byte

            // Calculate checksum in byte[14]: sum of bytes[0:14]
            uint8_t checksum = 0;
            for (int i = 0; i < 14; i++) {
                checksum += write[i];
            }
            write[14] = checksum;
        }

        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("forceResistance"));
        writeCharacteristic(write, sizeof(write), QStringLiteral("forceResistance"), false, true);
    }
}

uint16_t proformelliptical::watts() { return m_watt.value(); }
