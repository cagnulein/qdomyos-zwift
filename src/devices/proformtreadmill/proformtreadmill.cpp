#include "proformtreadmill.h"
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

proformtreadmill::proformtreadmill(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &proformtreadmill::update);
    refresh->start(200ms);
}

void proformtreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

void proformtreadmill::forceIncline(double incline) {
    if (proform_treadmill_1800i || proform_2000_treadmill) {
        uint8_t i = abs(incline * 10);
        uint8_t r = i % 5;
        i = i - r;
        if (incline > 0)
            incline = (double)i / 10.0;
        else
            incline = -((double)i / 10.0);
    }

    uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
    uint8_t write[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x04, 0x09, 0x02, 0x01,
                       0x02, 0xbc, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    write[12] = ((uint16_t)(incline * 100) >> 8) & 0xFF;
    write[11] = ((uint16_t)(incline * 100) & 0xFF);

    if (norditrack_s25i_treadmill) {
        write[14] = write[11] + write[12] + 0x11;
    } else if (proform_treadmill_8_0 || proform_treadmill_705_cst || proform_treadmill_9_0 || proform_treadmill_se || proform_treadmill_z1300i || proform_treadmill_l6_0s || norditrack_s25_treadmill || proform_8_5_treadmill || proform_2000_treadmill || proform_treadmill_sport_8_5 || proform_treadmill_505_cst) {
        write[14] = write[11] + write[12] + 0x12;
    } else if (!nordictrack_t65s_treadmill && !nordictrack_s30_treadmill && !nordictrack_s20_treadmill && !nordictrack_t65s_83_treadmill) {
        for (uint8_t i = 0; i < 7; i++) {
            write[14] += write[i + 6];
        }
    } else {
        write[14] = write[11] + 0x12;
    }

    writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("forceIncline"));
    writeCharacteristic(write, sizeof(write), QStringLiteral("forceIncline"), false, true);
}

void proformtreadmill::forceSpeed(double speed) {
    uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
    uint8_t write[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x04, 0x09, 0x02, 0x01,
                       0x01, 0xbc, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    write[12] = ((uint16_t)(speed * 100) >> 8) & 0xFF;
    write[11] = ((uint16_t)(speed * 100) & 0xFF);

    if (norditrack_s25i_treadmill) {
        write[14] = write[11] + write[12] + 0x11;
    } else if (proform_treadmill_8_0 || proform_treadmill_9_0 || proform_treadmill_se || proform_cadence_lt ||
               proform_treadmill_z1300i || proform_treadmill_l6_0s || norditrack_s25_treadmill || proform_8_5_treadmill || proform_2000_treadmill ||
               proform_treadmill_sport_8_5 || proform_treadmill_505_cst || proform_treadmill_705_cst) {
        write[14] = write[11] + write[12] + 0x11;
    } else if (!nordictrack_t65s_treadmill && !nordictrack_s30_treadmill && !nordictrack_s20_treadmill && !nordictrack_t65s_83_treadmill) {
        for (uint8_t i = 0; i < 7; i++) {
            write[14] += write[i + 6];
        }
    } else {
        write[14] = write[11] + 0x12;
    }

    writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("forceSpeed"));
    writeCharacteristic(write, sizeof(write), QStringLiteral("forceSpeed"), false, true);
}

void proformtreadmill::update() {
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
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        /*if (proform_treadmill_995i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                    if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                        requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }

                break;
            }
            counterPoll++;
            if (counterPoll > 2) {
                counterPoll = 0;
            }
        } else*/
        if (proform_treadmill_9_0 || proform_treadmill_z1300i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x92, 0x1a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0xC6, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), true);
                if (requestInclination != -100) {
                    if (requestInclination < 0)
                        requestInclination = 0;
                    if (requestInclination != currentInclination().value() && requestInclination >= -3 &&
                        requestInclination <= (proform_treadmill_z1300i ? 12 : 15)) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 &&
                        requestSpeed <= (proform_treadmill_z1300i ? 19.3 : 22)) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"), false, true);
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x35};
                    uint8_t start3[] = {0xff, 0x0e, 0x07, 0x00, 0x00, 0x6c, 0x20, 0x58, 0x02, 0x01,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0x76, 0x00, 0x00, 0x00, 0x00};
                    uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                    uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02,
                                        0x03, 0x10, 0xc8, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xfa, 0x00};
                    writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                    writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                    writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                    writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);

                    requestStart = -1;
                    emit tapeStarted();
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
        } else if (nordictrackt70 || proform_treadmill_sport_8_5) {
            uint8_t noOpData1[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData2[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData3[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};

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
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }

                break;
            case 4:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x30};
                    uint8_t start3[] = {0xff, 0x0e, 0x2a, 0x00, 0x00, 0xa0, 0x28, 0x58, 0x02, 0x01,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0xd0, 0x00, 0x00, 0x00, 0x00};
                    uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                    uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02,
                                        0x03, 0x10, 0xc8, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xfa, 0x00};
                    writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                    writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                    writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                    writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }

                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_2000_treadmill || proform_treadmill_705_cst) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                    if (requestInclination < -3 && proform_2000_treadmill)
                        requestInclination = -3;
                    else if (requestInclination < 0 && !proform_2000_treadmill)
                        requestInclination = 0;
                    if (requestInclination != currentInclination().value() && 
                            ((requestInclination >= -3 && proform_2000_treadmill) || 
                            (requestInclination >= 0 && !proform_2000_treadmill)) &&
                        requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), true);
                break;
            case 4:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));
                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }

                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }            
        } else if (nordictrack10) {
            uint8_t noOpData1[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData2[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData3[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};

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
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }

                break;
            case 4:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }

                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (nordictrack_t65s_treadmill || nordictrack_t65s_83_treadmill) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x81, 0x00, 0x10, 0x86, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00,
                                   0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), true);
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));

                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09, 0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0xca};
                    uint8_t start3[] = {0xff, 0x0e, 0x05, 0x00, 0x00, 0xc7, 0x20, 0x58, 0x02, 0x01, 0xb4, 0x00, 0x58, 0x02, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00};
                    uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                    uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02, 0x03, 0x10, 0xc8, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xfa, 0x00};
                    writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                    writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                    writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                    writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1 || requestPause != -1) {
                    forceSpeed(0);

                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                    requestPause = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_8_5_treadmill) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
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

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }            
        } else if (norditrack_s25i_treadmill) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x05, 0xed, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
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

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (nordictrack_s20_treadmill) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }

                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x30};
                    uint8_t start3[] = {0xff, 0x0e, 0x2a, 0x00, 0x00, 0xef, 0x1a, 0x58, 0x02, 0x00,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};
                    uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                    uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02,
                                        0x03, 0x10, 0xa0, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xd2, 0x00};
                    writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                    writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                    writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                    writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1 || requestPause != -1) {
                    forceSpeed(0);

                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                    requestPause = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }       
        } else if (proform_treadmill_505_cst) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }

                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x30};
                    uint8_t start3[] = {0xff, 0x0e, 0x2a, 0x00, 0x00, 0xef, 0x1a, 0x58, 0x02, 0x00,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};
                    uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                    uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02,
                                        0x03, 0x10, 0xa0, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xd2, 0x00};
                    writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                    writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                    writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                    writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1 || requestPause != -1) {
                    forceSpeed(0);

                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                    requestPause = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }                 
        } else if (proform_treadmill_l6_0s) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }

                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x30};
                    uint8_t start3[] = {0xff, 0x0e, 0x2a, 0x00, 0x00, 0xef, 0x1a, 0x58, 0x02, 0x00,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00};
                    uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                    uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02,
                                        0x03, 0x10, 0xa0, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xd2, 0x00};
                    writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                    writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                    writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                    writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1 || requestPause != -1) {
                    forceSpeed(0);

                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                    requestPause = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_treadmill_se) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x92, 0x98, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData7[] = {0xfe, 0x02, 0x10, 0x02};
            uint8_t noOpData8[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x04, 0x0c, 0x02, 0x02,
                                   0x00, 0x04, 0xa9, 0x00, 0x00, 0x04, 0x00, 0xc5, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
                if (requestInclination != -100) {
                    if (requestInclination < -3)
                        requestInclination = -3;
                    if (requestInclination != currentInclination().value() && requestInclination >= -3 &&
                        requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), true);
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));

                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            case 6:
                writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("noOp"));
                break;
            case 7:
                writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("noOp"), false, true);
                break;
            }
            counterPoll++;
            if (counterPoll > 7) {
                counterPoll = 0;
            }
        } else if (nordictrack_incline_trainer_x7i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x05, 0x13, 0x02, 0x00,
                                   0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x75, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x05, 0x13, 0x02, 0x00,
                                   0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x30, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                    if (requestInclination < -3)
                        requestInclination = -3;
                    if (requestInclination != currentInclination().value() && requestInclination >= -3 &&
                        requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), true);
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));

                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_treadmill_1800i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                    if (requestInclination < -3)
                        requestInclination = -3;
                    if (requestInclination != currentInclination().value() && requestInclination >= -3 &&
                        requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                break;
            case 3:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), true);
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));

                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (nordictrack_s30_treadmill) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00,
                                   0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_cadence_lt) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x91, 0x94, 0x31, 0x00, 0x00, 0x40, 0x00, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x50, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x11, 0x9e, 0x71, 0x00, 0x00, 0x40, 0x00, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0x87, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
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
        } else if (proform_treadmill_8_0) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x92, 0x1a, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x81, 0x00, 0x10, 0xb8, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00,
                                   0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), false, true);
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"), false, true);
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_pro_1000_treadmill) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x85, 0x00, 0x10, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00,
                                   0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), false, true);
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"), false, true);
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }            
        } else if (norditrack_s25_treadmill) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x85, 0x00, 0x10, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00, 0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};            

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
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), false, true);
                break;
            case 4:
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"), false, true);
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
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
                }
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }                
        } else {
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
                break;
            case 5:
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
                break;
            case 6:
                writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("noOp"));
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

                    // btinit();

                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 6) {
                counterPoll = 0;
            }
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }
    }
}

void proformtreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void proformtreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void proformtreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
        newValue.at(3) != 0x04 ||
        ((nordictrack10 || nordictrackt70 || proform_treadmill_1800i || proform_treadmill_z1300i || proform_treadmill_705_cst ||
          proform_treadmill_8_0 || proform_treadmill_9_0 || nordictrack_incline_trainer_x7i || proform_treadmill_sport_8_5 || proform_treadmill_505_cst) &&
         (newValue.at(4) != 0x02 || (newValue.at(5) != 0x31 && newValue.at(5) != 0x34))) ||
        ((norditrack_s25i_treadmill) && (newValue.at(4) != 0x02 || (newValue.at(5) != 0x2f))) ||
        ((nordictrack_t65s_treadmill || proform_pro_1000_treadmill || nordictrack_t65s_83_treadmill || nordictrack_s30_treadmill ||
          nordictrack_s20_treadmill || proform_treadmill_se || proform_cadence_lt || proform_8_5_treadmill) &&
         (newValue.at(4) != 0x02 || newValue.at(5) != 0x2e)) ||
        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) {
        return;
    }

    // filter some strange values from proform
    m_watts = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t)newValue.at(14)));

    // for the proform_treadmill_se this field is the distance in meters ;)
    if (m_watts > 3000 && !proform_treadmill_se) {
        m_watts = 0;
    } else {
        if (!proform_cadence_lt) {
            Inclination =
                (double)(((int16_t)((int16_t)newValue.at(13)) << 8) + (int16_t)((uint8_t)newValue.at(12))) / 100.0;
        }
        Speed = (double)(((uint16_t)((uint8_t)newValue.at(11)) << 8) + (uint16_t)((uint8_t)newValue.at(10))) / 100.0;
        if (watts(weight))
            KCal +=
                ((((0.048 * ((double)watts(weight)) + 1.19) * weight * 3.5) / 200.0) /
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

        cadenceFromAppleWatch();

        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
        // debug("Current Distance: " + QString::number(distance));
        emit debug(QStringLiteral("Current Watt: ") + QString::number(watts(weight)));

        if (m_control->error() != QLowEnergyController::NoError) {
            qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
        }
    }
}

void proformtreadmill::btinit() {
#ifdef Q_OS_WIN
    const int sleepms = 600;
#else
    const int sleepms = 400;
#endif    

    QSettings settings;    
    nordictrack10 =
        settings.value(QZSettings::nordictrack_10_treadmill, QZSettings::default_nordictrack_10_treadmill).toBool();
    nordictrackt70 =
        settings.value(QZSettings::nordictrack_t70_treadmill, QZSettings::default_nordictrack_t70_treadmill).toBool();
    nordictrack_t65s_treadmill =
        settings.value(QZSettings::nordictrack_t65s_treadmill, QZSettings::default_nordictrack_t65s_treadmill).toBool();
    nordictrack_s30_treadmill =
        settings.value(QZSettings::nordictrack_s30_treadmill, QZSettings::default_nordictrack_s30_treadmill).toBool();
    proform_treadmill_1800i =
        settings.value(QZSettings::proform_treadmill_1800i, QZSettings::default_proform_treadmill_1800i).toBool();
    proform_treadmill_se =
        settings.value(QZSettings::proform_treadmill_se, QZSettings::default_proform_treadmill_se).toBool();
    proform_treadmill_8_0 =
        settings.value(QZSettings::proform_treadmill_8_0, QZSettings::default_proform_treadmill_8_0).toBool();
    proform_treadmill_9_0 =
        settings.value(QZSettings::proform_treadmill_9_0, QZSettings::default_proform_treadmill_9_0).toBool();
    proform_cadence_lt =
        settings.value(QZSettings::proform_treadmill_cadence_lt, QZSettings::default_proform_treadmill_cadence_lt)
            .toBool();
    norditrack_s25i_treadmill =
        settings.value(QZSettings::norditrack_s25i_treadmill, QZSettings::default_norditrack_s25i_treadmill).toBool();
    norditrack_s25_treadmill =
        settings.value(QZSettings::norditrack_s25_treadmill, QZSettings::default_norditrack_s25_treadmill).toBool();        
    nordictrack_t65s_83_treadmill =
        settings.value(QZSettings::nordictrack_t65s_83_treadmill, QZSettings::default_nordictrack_t65s_83_treadmill)
            .toBool();
    nordictrack_incline_trainer_x7i =
        settings.value(QZSettings::nordictrack_incline_trainer_x7i, QZSettings::default_nordictrack_incline_trainer_x7i)
            .toBool();
    proform_treadmill_z1300i =
        settings.value(QZSettings::proform_treadmill_z1300i, QZSettings::default_proform_treadmill_z1300i).toBool();
    proform_pro_1000_treadmill =
        settings.value(QZSettings::proform_pro_1000_treadmill, QZSettings::default_proform_pro_1000_treadmill).toBool();
    nordictrack_s20_treadmill = settings.value(QZSettings::nordictrack_s20_treadmill,
                                                    QZSettings::default_nordictrack_s20_treadmill).toBool();
    proform_treadmill_l6_0s = settings.value(QZSettings::proform_treadmill_l6_0s, QZSettings::default_proform_treadmill_l6_0s).toBool();
    proform_8_5_treadmill = settings.value(QZSettings::proform_8_5_treadmill, QZSettings::default_proform_8_5_treadmill).toBool();
    proform_2000_treadmill = settings.value(QZSettings::proform_2000_treadmill, QZSettings::default_proform_2000_treadmill).toBool();
    proform_treadmill_sport_8_5 = settings.value(QZSettings::proform_treadmill_sport_8_5, QZSettings::default_proform_treadmill_sport_8_5).toBool();
    proform_treadmill_505_cst = settings.value(QZSettings::proform_treadmill_505_cst, QZSettings::default_proform_treadmill_505_cst).toBool();
    proform_treadmill_705_cst = settings.value(QZSettings::proform_treadmill_705_cst, QZSettings::default_proform_treadmill_705_cst).toBool();


    // bool proform_treadmill_995i = settings.value(QZSettings::proform_treadmill_995i,
    // QZSettings::default_proform_treadmill_995i).toBool();

    /*if (proform_treadmill_995i) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04,
                                0x00, 0xc1, 0x58, 0xfd, 0x90, 0x31, 0xd0, 0x75, 0x28, 0xc1};
        uint8_t initData11[] = {0x01, 0x12, 0x78, 0x2d, 0xc0, 0x71, 0x20, 0xf5, 0x88, 0x41,
                                0x18, 0xdd, 0x90, 0x51, 0x10, 0xd5, 0x88, 0x41, 0x38, 0xed};
        uint8_t initData12[] = {0xff, 0x08, 0xa0, 0x91, 0x40, 0x80, 0x02, 0x00, 0x00, 0x15,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else*/
    if (nordictrack10) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04,
                                0x00, 0x61, 0xd8, 0x5d, 0xd0, 0x51, 0xd0, 0x55, 0xe8, 0x61};
        uint8_t initData11[] = {0x01, 0x12, 0xf8, 0x8d, 0x00, 0x91, 0x20, 0xd5, 0x48, 0xe1,
                                0x98, 0x3d, 0xd0, 0x71, 0x10, 0xb5, 0x48, 0xe1, 0xb8, 0x4d};
        uint8_t initData12[] = {0xff, 0x08, 0xe0, 0xb1, 0x40, 0x80, 0x02, 0x00, 0x00, 0x75,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_8_5_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0x72, 0xf4, 0x74, 0xf2, 0x7e, 0x08, 0x80, 0x1e, 0xaa};
        uint8_t initData15[] = {0x01, 0x12, 0x3c, 0xcc, 0x5a, 0xe6, 0x90, 0x08, 0xa6, 0x42, 0xe4, 0x84, 0x22, 0xce, 0x98, 0x30, 0xce, 0x9a, 0x2c, 0xfc};
        uint8_t initData16[] = {0xff, 0x08, 0x8a, 0x56, 0x20, 0x98, 0x02, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
    } else if (proform_treadmill_sport_8_5) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0xd9, 0x78, 0x25, 0xc0, 0x69, 0x10, 0xcd, 0x78, 0x29};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData15[] = {0x01, 0x12, 0xd8, 0x85, 0x30, 0xf9, 0xa0, 0x7d, 0x38, 0xf9, 0xb8, 0x65, 0x20, 0xe9, 0xd0, 0x8d, 0x78, 0x29, 0x18, 0xc5};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData16[] = {0xff, 0x08, 0xb0, 0x99, 0x40, 0x80, 0x02, 0x00, 0x00, 0xbd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData20[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData22[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData23[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData25[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_2000_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0xb9, 0xf8, 0x45, 0x80, 0xc9, 0x10, 0x6d, 0xb8, 0x09};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData15[] = {0x01, 0x12, 0x58, 0xa5, 0xf0, 0x59, 0xa0, 0x1d, 0x78, 0xd9, 0x38, 0x85, 0xe0, 0x49, 0xd0, 0x2d, 0xb8, 0x09, 0x98, 0xe5};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData16[] = {0xff, 0x08, 0x70, 0xf9, 0x40, 0x80, 0x02, 0x00, 0x00, 0xdd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData17[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData19[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_pro_1000_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0x13, 0xc0, 0x7f, 0x34, 0xeb, 0xa0, 0x6f, 0x2c, 0xe3};
        uint8_t initData11[] = {0x01, 0x12, 0xa0, 0x7f, 0x34, 0x0b, 0xc0, 0x8f, 0x7c, 0x53, 0x00, 0xff, 0xd4, 0x8b, 0x60, 0x4f, 0x2c, 0x03, 0xe0, 0xff};
        uint8_t initData12[] = {0xff, 0x08, 0xd4, 0xab, 0x80, 0x90, 0x02, 0x00, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);       
    } else if (proform_treadmill_705_cst) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0x96, 0x24, 0xa8, 0x3a, 0xca, 0x68, 0xfc, 0x86, 0x2e};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData15[] = {0x01, 0x12, 0xcc, 0x50, 0xf2, 0x92, 0x50, 0xf4, 0x9e, 0x26, 0xf4, 0x98, 0x2a, 0xfa, 0xb8, 0x4c, 0x16, 0xde, 0x9c, 0x20};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData16[] = {0xff, 0x08, 0xe2, 0xa2, 0xa0, 0x80, 0x02, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData17[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData19[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData20[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData22[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData23[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData25[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_z1300i) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04,
                                0x00, 0xb0, 0x4c, 0xda, 0x7e, 0x14, 0xb8, 0x46, 0xfa, 0x98};
        uint8_t initData11[] = {0x01, 0x12, 0x34, 0xd2, 0x86, 0x3c, 0xf0, 0x9e, 0x52, 0xe0,
                                0xbc, 0x4a, 0x0e, 0xc4, 0x88, 0x56, 0x0a, 0xc8, 0x84, 0x42};
        uint8_t initData12[] = {0xff, 0x08, 0x36, 0xec, 0xe0, 0x80, 0x02, 0x00, 0x00, 0x12,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_incline_trainer_x7i) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x05, 0x04, 0x80, 0x89,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x05, 0x04, 0x88, 0x91,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x05, 0x28, 0x90, 0x04,
                                0x00, 0x59, 0x78, 0xa5, 0xc0, 0xe9, 0x10, 0x4d, 0x78, 0xa9};
        uint8_t initData11[] = {0x01, 0x12, 0xd8, 0x05, 0x30, 0x79, 0xa0, 0xfd, 0x38, 0x79,
                                0xb8, 0xe5, 0x20, 0x69, 0xd0, 0x0d, 0x78, 0xa9, 0x18, 0x45};
        uint8_t initData12[] = {0xff, 0x08, 0xb0, 0x19, 0x40, 0x80, 0x02, 0x00, 0x00, 0x3e,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x05, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa6, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x05, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3b, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (norditrack_s25_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0x38, 0xac, 0x12, 0x8e, 0xfc, 0x78, 0xee, 0x6a, 0xd0};
        uint8_t initData15[] = {0x01, 0x12, 0x54, 0xda, 0x56, 0xd4, 0x70, 0xf6, 0x62, 0xe8, 0x9c, 0x02, 0xbe, 0x2c, 0xc8, 0x7e, 0x1a, 0x80, 0x24, 0xca};
        uint8_t initData16[] = {0xff, 0x08, 0x66, 0x04, 0xe0, 0x98, 0x02, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
    } else if (norditrack_s25i_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0xe5, 0x88, 0x31, 0xd8, 0x8d, 0x30, 0xe1, 0x90, 0x45};
        uint8_t initData11[] = {0x01, 0x12, 0x08, 0xb1, 0x78, 0x3d, 0xe0, 0xa1, 0x60, 0x25,
                                0xe8, 0xd1, 0x98, 0x4d, 0x30, 0xe1, 0xd0, 0x85, 0x68, 0x51};
        uint8_t initData12[] = {0xff, 0x08, 0x38, 0x1d, 0xc0, 0x98, 0x02, 0x00, 0x00, 0x8d,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_cadence_lt) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0xe2, 0x34, 0x84, 0xd2, 0x2e, 0x88, 0xd0, 0x3e, 0x9a};
        uint8_t initData11[] = {0x01, 0x12, 0xfc, 0x5c, 0xba, 0x16, 0x90, 0xf8, 0x46, 0xd2,
                                0x24, 0xb4, 0x02, 0x9e, 0x18, 0x60, 0xee, 0x6a, 0xec, 0x6c};
        uint8_t initData12[] = {0xff, 0x08, 0xea, 0x66, 0x20, 0x98, 0x02, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_8_0) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0xe2, 0x34, 0x84, 0xd2, 0x2e, 0x88, 0xd0, 0x3e, 0x9a};
        uint8_t initData11[] = {0x01, 0x12, 0xfc, 0x5c, 0xba, 0x16, 0x90, 0xf8, 0x46, 0xd2,
                                0x24, 0xb4, 0x02, 0x9e, 0x18, 0x60, 0xee, 0x6a, 0xec, 0x6c};
        uint8_t initData12[] = {0xff, 0x08, 0xea, 0x66, 0x20, 0x98, 0x02, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrackt70) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04,
                                0x00, 0xc5, 0x08, 0x51, 0x98, 0xed, 0x30, 0x81, 0xd0, 0x25};
        uint8_t initData11[] = {0x01, 0x12, 0x88, 0xd1, 0x38, 0x9d, 0xe0, 0x41, 0xa0, 0x05,
                                0x68, 0xf1, 0x58, 0xad, 0x30, 0x81, 0x10, 0x65, 0xe8, 0x71};
        uint8_t initData12[] = {0xff, 0x08, 0xf8, 0x7d, 0xc0, 0x80, 0x02, 0x00, 0x00, 0x91,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_9_0) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04,
                                0x00, 0x91, 0x18, 0xad, 0x30, 0xc1, 0x50, 0xe5, 0x88, 0x11};
        uint8_t initData11[] = {0x01, 0x12, 0xb8, 0x5d, 0xe0, 0x81, 0x20, 0xc5, 0x68, 0x31,
                                0xd8, 0x6d, 0x30, 0xc1, 0x90, 0x25, 0xe8, 0xb1, 0x78, 0x3d};
        uint8_t initData12[] = {0xff, 0x08, 0xc0, 0x81, 0x40, 0x80, 0x02, 0x00, 0x00, 0x05,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData10[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData11[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData12[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData12, sizeof(noOpData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_l6_0s) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0xee, 0x44, 0x90, 0xea, 0x42, 0xa8, 0xf4, 0x56, 0xb6};
        uint8_t initData11[] = {0x01, 0x12, 0x2c, 0x88, 0xe2, 0x5a, 0xd0, 0x3c, 0x8e, 0x1e, 0x94, 0xe0, 0x7a, 0xf2, 0x78, 0xc4, 0x46, 0xc6, 0x7c, 0xf8};
        uint8_t initData12[] = {0xff, 0x08, 0x72, 0xea, 0xa0, 0x80, 0x02, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_t65s_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0xce, 0xc4, 0xb0, 0xaa, 0xa2, 0xa8, 0x94, 0x96, 0x96};
        uint8_t initData11[] = {0x01, 0x12, 0xac, 0xa8, 0xa2, 0xba, 0xd0, 0xdc, 0xce, 0xfe,
                                0x14, 0x00, 0x3a, 0x52, 0x78, 0x64, 0x86, 0xa6, 0xfc, 0x18};
        uint8_t initData12[] = {0xff, 0x08, 0x32, 0x4a, 0xa0, 0x88, 0x02, 0x00, 0x00, 0x44,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                               0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_t65s_83_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0xf4, 0xfc, 0xfe, 0x06, 0x00, 0x18, 0x12, 0x22, 0x3c};
        uint8_t initData11[] = {0x01, 0x12, 0x44, 0x56, 0x7e, 0x88, 0xb0, 0xaa, 0xca, 0xe4,
                                0x0c, 0x2e, 0x76, 0x90, 0xa8, 0xc2, 0x12, 0x2c, 0x74, 0x86};
        uint8_t initData12[] = {0xff, 0x08, 0xce, 0x18, 0x60, 0x98, 0x02, 0x00, 0x00, 0xaa,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_1800i) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0x92, 0x74, 0x54, 0x32, 0x1e, 0x08, 0xe0, 0xde, 0xca};
        uint8_t initData11[] = {0x01, 0x12, 0xbc, 0xac, 0x9a, 0x86, 0x90, 0x68, 0x66, 0x62,
                                0x64, 0x64, 0x62, 0x6e, 0x98, 0x90, 0x8e, 0xba, 0xac, 0xdc};
        uint8_t initData12[] = {0xff, 0x08, 0xca, 0xf6, 0x20, 0x80, 0x02, 0x00, 0x00, 0x38,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_se) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0x53, 0xc0, 0x3f, 0xb4, 0x2b, 0xa0, 0x2f, 0xac, 0x23};
        uint8_t initData11[] = {0x01, 0x12, 0xa0, 0x3f, 0xb4, 0x4b, 0xc0, 0x4f, 0xfc, 0x93,
                                0x00, 0xbf, 0x54, 0xcb, 0x60, 0x0f, 0xac, 0x43, 0xe0, 0xbf};
        uint8_t initData12[] = {0xff, 0x08, 0x54, 0xeb, 0x80, 0x88, 0x02, 0x00, 0x00, 0x33,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_s20_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 
                                0x90, 0x07, 0x01, 0x3b, 0xa0, 0x17, 0x84, 0xf3, 
                                0x60, 0xd7, 0x5c, 0xdb};
        uint8_t initData11[] = {0x01, 0x12, 0x40, 0xc7, 0x44, 0xc3, 0x40, 0xc7, 
                                0x4c, 0xfb, 0x60, 0x17, 0x84, 0x33, 0xa0, 0x57, 
                                0xfc, 0x9b, 0x00, 0xa7};
        uint8_t initData12[] = {0xff, 0x08, 0x44, 0xe3, 0x80, 0x88, 0x02, 0x00, 
                                0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = { 0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 
                                0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c,
                                0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 
                                0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);        
    } else if (nordictrack_s30_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0x64, 0x3c, 0x0e, 0xe6, 0xb0, 0x98, 0x62, 0x42, 0x2c};
        uint8_t initData11[] = {0x01, 0x12, 0x04, 0xe6, 0xde, 0xb8, 0xb0, 0x9a, 0x6a, 0x74,
                                0x4c, 0x5e, 0x56, 0x20, 0x28, 0x32, 0x32, 0x3c, 0x34, 0x36};
        uint8_t initData12[] = {0xff, 0x08, 0x2e, 0x28, 0x60, 0x98, 0x02, 0x00, 0x00, 0xba,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_505_cst) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0xe1, 0xd8, 0xdd, 0xd0, 0xd1, 0xd0, 0xd5, 0xe8, 0xe1};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData15[] = {0x01, 0x12, 0xf8, 0x0d, 0x00, 0x11, 0x20, 0x55, 0x48, 0x61, 0x98, 0xbd, 0xd0, 0xf1, 0x10, 0x35, 0x48, 0x61, 0xb8, 0xcd};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData16[] = {0xff, 0x08, 0xe0, 0x31, 0x40, 0x80, 0x02, 0x00, 0x00, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData20[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData22[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData23[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData25[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else {
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
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07,
                                0x01, 0x10, 0xcc, 0x7a, 0x3e, 0xf4, 0xb8, 0x66, 0x3a, 0xf8};
        uint8_t initData11[] = {0x01, 0x12, 0xb4, 0x72, 0x46, 0x1c, 0xf0, 0xbe, 0x92, 0x40,
                                0x3c, 0xea, 0xce, 0xa4, 0x88, 0x76, 0x4a, 0x28, 0x04, 0xe2};
        uint8_t initData12[] = {0xff, 0x08, 0xf6, 0xcc, 0xe0, 0x98, 0x02, 0x00, 0x00, 0xd1,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
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
    }

    initDone = true;
}

void proformtreadmill::stateChanged(QLowEnergyService::ServiceState state) {
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
                &proformtreadmill::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &proformtreadmill::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &proformtreadmill::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &proformtreadmill::descriptorWritten);

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
                    emit debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &proformtreadmill::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &proformtreadmill::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    emit debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &proformtreadmill::changeInclinationRequested);
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

void proformtreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void proformtreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void proformtreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &proformtreadmill::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void proformtreadmill::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("proformtreadmill::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformtreadmill::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("proformtreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformtreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_TL")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &proformtreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &proformtreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &proformtreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &proformtreadmill::controllerStateChanged);

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

bool proformtreadmill::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void proformtreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
