#include "proformtreadmill.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include "homeform.h"
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
    m_watt.setType(metric::METRIC_WATT, deviceType());
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
    } else if (proform_treadmill_8_0 || proform_treadmill_705_cst || proform_treadmill_705_cst_V78_239 || proform_treadmill_9_0 || proform_treadmill_se ||
                proform_treadmill_z1300i || proform_treadmill_l6_0s || norditrack_s25_treadmill || proform_8_5_treadmill || nordictrack_treadmill_exp_5i || proform_2000_treadmill ||
                proform_treadmill_sport_8_5 || proform_treadmill_505_cst || proform_505_cst_80_44 || proform_carbon_tl || proform_proshox2 || nordictrack_s20i_treadmill || proform_595i_proshox2 ||
               proform_treadmill_8_7 || proform_carbon_tl_PFTL59720 || proform_treadmill_sport_70 || proform_treadmill_575i || proform_performance_300i || proform_performance_400i || proform_treadmill_c700 ||
               proform_treadmill_c960i || nordictrack_tseries5_treadmill || proform_carbon_tl_PFTL59722c || proform_treadmill_1500_pro || proform_trainer_8_0 || proform_treadmill_705_cst_V80_44 ||
               nordictrack_treadmill_ultra_le || proform_treadmill_carbon_tls || proform_treadmill_sport_3_0 || proform_treadmill_995i || nordictrack_series_7
               ) {
        write[14] = write[11] + write[12] + 0x12;
    } else if (!nordictrack_t65s_treadmill && !nordictrack_elite_800 && !nordictrack_t65s_treadmill_81_miles && !nordictrack_s30_treadmill && !nordictrack_s20_treadmill && !nordictrack_t65s_83_treadmill) {
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
               proform_treadmill_z1300i || proform_treadmill_l6_0s || norditrack_s25_treadmill || proform_8_5_treadmill || nordictrack_treadmill_exp_5i || proform_2000_treadmill ||
               proform_treadmill_sport_8_5 || proform_treadmill_505_cst || proform_505_cst_80_44 || proform_treadmill_705_cst || proform_treadmill_705_cst_V78_239 || proform_carbon_tl || proform_proshox2 || nordictrack_s20i_treadmill || proform_595i_proshox2 ||
               proform_treadmill_8_7 || proform_carbon_tl_PFTL59720 || proform_treadmill_sport_70 || proform_treadmill_575i || proform_performance_300i || proform_performance_400i || proform_treadmill_c700 ||
               proform_treadmill_c960i || nordictrack_tseries5_treadmill || proform_carbon_tl_PFTL59722c || proform_treadmill_1500_pro || proform_trainer_8_0 || proform_treadmill_705_cst_V80_44 ||
               nordictrack_treadmill_ultra_le || proform_treadmill_carbon_tls || proform_treadmill_sport_3_0 || proform_treadmill_995i || nordictrack_series_7) {
        write[14] = write[11] + write[12] + 0x11;
    } else if (!nordictrack_t65s_treadmill && !nordictrack_elite_800 && !nordictrack_t65s_treadmill_81_miles && !nordictrack_s30_treadmill && !nordictrack_s20_treadmill && !nordictrack_t65s_83_treadmill) {
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
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

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
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x30};
                    uint8_t start3[] = {0xff, 0x0e, 0x2a, 0x00, 0x00, 0x6c, 0x20, 0x58, 0x02, 0x01,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00};
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
                if (requestStop != -1 || requestPause != -1) {
                    uint8_t stop1[] = {0xfe, 0x02, 0x0f, 0x02};
                    uint8_t stop2[] = {0xff, 0x0f, 0x02, 0x04, 0x02, 0x0b, 0x04, 0x0b, 0x02, 0x02,
                                       0x02, 0x10, 0x00, 0x00, 0x01, 0x00, 0x26, 0x00, 0x00, 0x00};
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop1"));
                    writeCharacteristic(stop2, sizeof(stop2), QStringLiteral("stop2"));

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
        } else if (proform_treadmill_carbon_tls) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x05, 0xed, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00,
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
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "start tape");
                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
                    requestStop = -1;
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
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (nordictrack_treadmill_ultra_le) {
            // Update function frames for NordicTrack Treadmill Ultra LE
            // These frames should be used in the update/polling function

            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03}; // Frame from pkt5293 - for update function
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80}; // Frame from pkt5301 - for update function
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x05, 0xed, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5306 - for update function
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03}; // Frame from pkt5319 - for update function
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5326 - for update function
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5333 - for update function

            // These frames (pkt5343, pkt5352, pkt5357, pkt5367, pkt5378, pkt5384)
            // are duplicates of previous frames and would be used in polling cycle

            // Polling cycle implementation for update function
            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);

                // Handle speed and inclination requests during this polling cycle
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

                // Handle start and stop requests during this polling cycle
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));
                    // btinit(); // Uncomment if re-initialization is needed on start
                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    // Add specific stop command if needed for Ultra LE
                    requestStop = -1;
                }
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_treadmill_705_cst_V80_44) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b,
                                   0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80,
                                   0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        } else if (nordictrack_series_7) {
            // NordicTrack Series 7 sendPoll cycle (6 packets)
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15,
                                   0x02, 0x00, 0x0f, 0x80, 0x02, 0x40, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x81, 0x00, 0x10,
                                   0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10,
                                   0x02, 0x00, 0x0a, 0x13, 0x94, 0x33, 0x00, 0x10,
                                   0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x02, 0x18, 0xf2, 0x00, 0x00, 0x00, 0x00,
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
                    if (requestInclination != currentInclination().value() &&
                        requestInclination >= 0 && requestInclination <= 15) {
                        emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() &&
                        requestSpeed >= 0 && requestSpeed <= 22) {
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
        } else if (proform_treadmill_1500_pro) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00, 
                                0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 
                                0x0f, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x85, 0x00, 0x10, 0x8a, 0x00,
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
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));
                    requestStart = -1;
                    emit tapeStarted();
                }
                if (requestStop != -1) {
                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
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
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }            
        } else if (nordictrack_tseries5_treadmill) {
            // Frame 1
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            // Frame 2
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            // Frame 3
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            // Frame 4
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            // Frame 5
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            // Frame 6
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00,
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
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"), false, true);
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
        } else if (proform_treadmill_c960i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00,
                                   0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x85, 0x00, 0x10, 0x8a, 0x00,
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
        } else if (proform_treadmill_995i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00,
                                   0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                   0x0f, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x85, 0x00, 0x10, 0x8a, 0x00,
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
        } else if (proform_treadmill_sport_3_0) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00, 0x0a, 0x13, 0x94, 0x33, 0x00, 0x10, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x02, 0x18, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x81, 0x00, 0x10, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            
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
                        requestInclination <= 20) {
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
        } else if (proform_2000_treadmill || proform_treadmill_705_cst_V78_239) {																				 																									  
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
        } else if (nordictrack_t65s_treadmill || nordictrack_t65s_83_treadmill || nordictrack_t65s_treadmill_81_miles) {
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

                    if(nordictrack_t65s_treadmill_81_miles) {
                        uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                        uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09, 0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x30};
                        uint8_t start3[] = {0xff, 0x0e, 0x2a, 0x00, 0x00, 0xc7, 0x20, 0x58, 0x02, 0x00, 0xb4, 0x00, 0x58, 0x02, 0x00, 0xee, 0x00, 0x00, 0x00, 0x00};
                        uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                        uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02, 0x03, 0x10, 0xa0, 0x00, 0x00, 0x00, 0x0a, 0x00, 0xd2, 0x00};
                        writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                        writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                        writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                        writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                        writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);
                    } else {
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
                    }

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
        } else if (proform_8_5_treadmill || nordictrack_treadmill_exp_5i) {
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
        } else if (nordictrack_s20i_treadmill) {
            unsigned char array1[] = {0xfe, 0x02, 0x17, 0x03};
            unsigned char array2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            unsigned char array3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            unsigned char array4[] = {0xfe, 0x02, 0x19, 0x03};
            unsigned char array5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            unsigned char array6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


            switch (counterPoll) {
            case 0:
                writeCharacteristic(array1, sizeof(array1), QStringLiteral("noOp"));
                break;
            case 1:
                writeCharacteristic(array2, sizeof(array2), QStringLiteral("noOp"));
                break;
            case 2:
                writeCharacteristic(array3, sizeof(array3), QStringLiteral("noOp"), false, true);
                break;
            case 3:
                writeCharacteristic(array4, sizeof(array4), QStringLiteral("noOp"));
                break;
            case 4:
                writeCharacteristic(array5, sizeof(array5), QStringLiteral("noOp"));
                break;
            case 5:
                writeCharacteristic(array6, sizeof(array6), QStringLiteral("noOp"));
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
        } else if (proform_treadmill_705_cst) {
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
                        emit debug(QStringLiteral("writing incline = ") + QString::number(requestInclination));
                        forceIncline(requestInclination);
                    }
                    requestInclination = -100;
                }

//              Request start does not seem to get initiated automatically.
//              Instead, trigger it when the treadmill is stopped and a new speed is set via the application.
//              Note: Setting a speed directly on the treadmill will not work until the initiate commands are sent.
                if (Distance.value() == 0 && currentSpeed().value() == 0 && requestSpeed > 0) {
                    requestStart = 1;
                    emit debug(QStringLiteral("Request start = ") + QString::number(requestSpeed) + QStringLiteral("kph"));
                }

//				Initiate treadmill to start
                if (requestStart != -1) {
                    emit debug(QStringLiteral("starting..."));

//                  Define treadmill settings
                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x30};
                    uint8_t start3[] = {0xff, 0x0e, 0x2a, 0x00, 0x00, 0xc7, 0x20, 0x58, 0x02, 0x01,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0xef, 0x00, 0x00, 0x00, 0x00};

//		    		Start treadmill session (speed=2kph 00c8  incline=1 0064)
                    uint8_t start4[] = {0xfe, 0x02, 0x11, 0x02};
                    uint8_t start5[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x02,
                                        0x03, 0x10, 0xc8, 0x00, 0x64, 0x00, 0x02, 0x00, 0x8e, 0x00};

                    writeCharacteristic(start1, sizeof(start1), QStringLiteral("start1"));
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start2"));
                    writeCharacteristic(start3, sizeof(start3), QStringLiteral("start3"), false, true);
                    writeCharacteristic(start4, sizeof(start4), QStringLiteral("start4"));
                    writeCharacteristic(start5, sizeof(start5), QStringLiteral("start5"), false, true);

                    requestStart = -1;
                    emit tapeStarted();
                }

//				Set treadmill speed
                if (requestSpeed != -1) {
                    if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                        emit debug(QStringLiteral("writing speed = ") + QString::number(requestSpeed) + QStringLiteral("kph"));
                        forceSpeed(requestSpeed);
                    }
                    requestSpeed = -1;
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
        } else if (proform_carbon_tl) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x92, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
        } else if (nordictrack_elite_800) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x92, 0x18, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0xd4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData6[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x05, 0xed, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        } else if (proform_carbon_tl_PFTL59720) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x0a, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x81, 0x00, 0x10, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
        } else if (proform_treadmill_8_7) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        } else if (proform_carbon_tl_PFTL59722c) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        } else if (proform_595i_proshox2) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x92, 0x9a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        } else if (proform_treadmill_sport_70) {
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
        } else if (proform_treadmill_575i) {
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
        } else if (proform_proshox2) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x92, 0x9a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        } else if (proform_trainer_8_0) {
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
        } else if (proform_treadmill_c700) {
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
                if (requestStop != -1 || requestPause != -1) {
                    uint8_t stop1[] = {0xfe, 0x02, 0x0f, 0x02};
                    uint8_t stop2[] = {0xff, 0x0f, 0x02, 0x04, 0x02, 0x0b, 0x04, 0x0b, 0x02, 0x02,
                                       0x02, 0x10, 0x00, 0x00, 0x01, 0x00, 0x26, 0x00, 0x00, 0x00};
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop1"));
                    writeCharacteristic(stop2, sizeof(stop2), QStringLiteral("stop2"), false, true);

                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                    requestPause = -1;
                }            
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

                    uint8_t start1[] = {0xfe, 0x02, 0x20, 0x03};
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x08};
                    uint8_t start3[] = {0xff, 0x0e, 0x07, 0x00, 0x00, 0x14, 0x1e, 0x58, 0x02, 0x01,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0xef, 0x00, 0x00, 0x00, 0x00};
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
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }        
        } else if (proform_performance_300i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                if (requestStop != -1 || requestPause != -1) {
                    uint8_t stop1[] = {0xfe, 0x02, 0x0f, 0x02};
                    uint8_t stop2[] = {0xff, 0x0f, 0x02, 0x04, 0x02, 0x0b, 0x04, 0x0b, 0x02, 0x02,
                                       0x02, 0x10, 0x00, 0x00, 0x01, 0x00, 0x26, 0x00, 0x00, 0x00};
                                       
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop1"));
                    writeCharacteristic(stop2, sizeof(stop2), QStringLiteral("stop2"), false, true);

                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                    requestPause = -1;
                }
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
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x08};
                    uint8_t start3[] = {0xff, 0x0e, 0x05, 0x00, 0x00, 0x60, 0x22, 0x58, 0x02, 0x01,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00};
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
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_performance_400i) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData3[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            switch (counterPoll) {
            case 0:
                if (requestStop != -1 || requestPause != -1) {
                    uint8_t stop1[] = {0xfe, 0x02, 0x0f, 0x02};
                    uint8_t stop2[] = {0xff, 0x0f, 0x02, 0x04, 0x02, 0x0b, 0x04, 0x0b, 0x02, 0x02,
                                       0x02, 0x10, 0x00, 0x00, 0x01, 0x00, 0x26, 0x00, 0x00, 0x00};
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop1"));
                    writeCharacteristic(stop2, sizeof(stop2), QStringLiteral("stop2"), false, true);

                    emit debug(QStringLiteral("stopping..."));
                    requestStop = -1;
                    requestPause = -1;
                }
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
                    uint8_t start2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x1c, 0x04, 0x1c, 0x02, 0x09,
                                        0x00, 0x00, 0x40, 0x02, 0x18, 0x40, 0x00, 0x00, 0x80, 0x08};
                    uint8_t start3[] = {0xff, 0x0e, 0x07, 0x00, 0x00, 0x14, 0x1e, 0x58, 0x02, 0x01,
                                        0xb4, 0x00, 0x58, 0x02, 0x00, 0xef, 0x00, 0x00, 0x00, 0x00};
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
                break;
            }
            counterPoll++;
            if (counterPoll > 5) {
                counterPoll = 0;
            }
        } else if (proform_505_cst_80_44) {
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                   0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00,
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
    
    if(!initDone) {
        qDebug() << "discarding, init in progress...";
        return;
    }

    if (proform_treadmill_705_cst && currentSpeed().value() == 0)
    {
        QString hexString = newValue.toHex(' ');
        if (hexString.startsWith("fe 02 25 04 00 00") && newValue.at(17) == 0x01)
        {
            emit debug(QStringLiteral("start button pressed on console!"));
	    if(!homeform::singleton())
            	Distance = 0;
            requestStart = 1;
            return;
        }
    }

    // Ignore packets with all zeros in data bytes (no useful information)
    // This fixes the issue where speed oscillates between actual value and 0
    if (proform_treadmill_sport_70 && newValue.length() == 20 &&
        newValue.at(0) == 0x00 && newValue.at(1) == 0x12 && newValue.at(2) == 0x01 && newValue.at(3) == 0x04 &&
        newValue.at(5) == 0x1c &&
        newValue.at(10) == 0x00 && newValue.at(11) == 0x00 && newValue.at(12) == 0x00 && newValue.at(13) == 0x00 &&
        newValue.at(14) == 0x00 && newValue.at(15) == 0x00 && newValue.at(16) == 0x00 && newValue.at(17) == 0x00 &&
        newValue.at(18) == 0x00 && newValue.at(19) == 0x00) {
        return;
    }

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
        newValue.at(3) != 0x04 ||

        ((nordictrack10 || nordictrackt70 || proform_treadmill_1800i || proform_treadmill_z1300i || proform_treadmill_705_cst || proform_treadmill_705_cst_V78_239 ||
          proform_treadmill_8_0 || proform_treadmill_9_0 || nordictrack_incline_trainer_x7i || proform_treadmill_sport_8_5 || proform_treadmill_505_cst || proform_505_cst_80_44 ||
          proform_proshox2 || proform_595i_proshox2 || proform_performance_300i || proform_performance_400i || proform_treadmill_705_cst_V80_44) &&
         (newValue.at(4) != 0x02 || (newValue.at(5) != 0x31 && newValue.at(5) != 0x34))) ||

        ((norditrack_s25i_treadmill || nordictrack_treadmill_ultra_le || proform_treadmill_carbon_tls) && (newValue.at(4) != 0x02 || (newValue.at(5) != 0x2f))) ||

        ((nordictrack_t65s_treadmill || nordictrack_t65s_treadmill_81_miles || proform_pro_1000_treadmill || nordictrack_t65s_83_treadmill || nordictrack_s30_treadmill ||
          nordictrack_s20_treadmill || proform_treadmill_se || proform_cadence_lt || proform_8_5_treadmill || nordictrack_treadmill_exp_5i || proform_carbon_tl ||
          nordictrack_s20i_treadmill || proform_treadmill_8_7 || proform_carbon_tl_PFTL59720 || proform_treadmill_575i || nordictrack_tseries5_treadmill ||
          proform_carbon_tl_PFTL59722c || proform_treadmill_1500_pro || proform_treadmill_995i) &&
         (newValue.at(4) != 0x02 || newValue.at(5) != 0x2e)) ||

        ((nordictrack_elite_800) &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF) ||

        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF) ||

        (((((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) && 
            (proform_proshox2 || proform_595i_proshox2 || proform_treadmill_sport_3_0))
    )
    {
        return;
    }

    // filter some strange values from proform
    m_watts = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t)newValue.at(14)));

    // for the proform_treadmill_se this field is the distance in meters ;)
    if (m_watts > 3000 && !proform_treadmill_se && !nordictrack_s20i_treadmill && !nordictrack_tseries5_treadmill && !proform_treadmill_sport_3_0) {
        m_watts = 0;
    } else {
        if (!proform_cadence_lt) {
            Inclination =
                (double)(((int16_t)((int8_t)newValue.at(13)) << 8) + (int16_t)((uint8_t)newValue.at(12))) / 100.0;
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

        if(!cadenceFromAppleWatch() && Speed.value() > 0) {
            bool hasPowerSensor = !settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                                    .toString()
                                    .startsWith(QStringLiteral("Disabled"));
            if (!hasPowerSensor) {
                double calculatedCadence = calculateCadenceFromSpeed(Speed.value());
                if (calculatedCadence > 0) {
                    evaluateStepCount();
                    Cadence = calculatedCadence;
                    emit debug(QStringLiteral("Current Cadence (calculated from speed): ") + QString::number(Cadence.value()));
                }
            }
        }

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
    nordictrack_treadmill_exp_5i = settings.value(QZSettings::nordictrack_treadmill_exp_5i, QZSettings::default_nordictrack_treadmill_exp_5i).toBool();
    proform_pro_1000_treadmill =
        settings.value(QZSettings::proform_pro_1000_treadmill, QZSettings::default_proform_pro_1000_treadmill).toBool();
    nordictrack_s20_treadmill = settings.value(QZSettings::nordictrack_s20_treadmill,
                                                    QZSettings::default_nordictrack_s20_treadmill).toBool();
    nordictrack_s20i_treadmill = settings.value(QZSettings::nordictrack_s20i_treadmill,
                                               QZSettings::default_nordictrack_s20i_treadmill).toBool();
    proform_treadmill_l6_0s = settings.value(QZSettings::proform_treadmill_l6_0s, QZSettings::default_proform_treadmill_l6_0s).toBool();
    proform_8_5_treadmill = settings.value(QZSettings::proform_8_5_treadmill, QZSettings::default_proform_8_5_treadmill).toBool();
    proform_2000_treadmill = settings.value(QZSettings::proform_2000_treadmill, QZSettings::default_proform_2000_treadmill).toBool();
    proform_treadmill_sport_8_5 = settings.value(QZSettings::proform_treadmill_sport_8_5, QZSettings::default_proform_treadmill_sport_8_5).toBool();
    proform_treadmill_505_cst = settings.value(QZSettings::proform_treadmill_505_cst, QZSettings::default_proform_treadmill_505_cst).toBool();
    proform_treadmill_705_cst = settings.value(QZSettings::proform_treadmill_705_cst, QZSettings::default_proform_treadmill_705_cst).toBool();
    proform_treadmill_705_cst_V78_239 = settings.value(QZSettings::proform_treadmill_705_cst_V78_239, QZSettings::default_proform_treadmill_705_cst_V78_239).toBool();
    proform_carbon_tl = settings.value(QZSettings::proform_carbon_tl, QZSettings::default_proform_carbon_tl).toBool();
    proform_proshox2 = settings.value(QZSettings::proform_proshox2, QZSettings::default_proform_proshox2).toBool();
    proform_595i_proshox2 = settings.value(QZSettings::proform_595i_proshox2, QZSettings::default_proform_595i_proshox2).toBool();
    proform_treadmill_8_7 = settings.value(QZSettings::proform_treadmill_8_7, QZSettings::default_proform_treadmill_8_7).toBool();
    proform_carbon_tl_PFTL59720 = settings.value(QZSettings::proform_carbon_tl_PFTL59720, QZSettings::default_proform_carbon_tl_PFTL59720).toBool();
    proform_treadmill_sport_70 = settings.value(QZSettings::proform_treadmill_sport_70, QZSettings::default_proform_treadmill_sport_70).toBool();
    proform_treadmill_575i = settings.value(QZSettings::proform_treadmill_575i, QZSettings::default_proform_treadmill_575i).toBool();
    proform_performance_300i = settings.value(QZSettings::proform_performance_300i, QZSettings::default_proform_performance_300i).toBool();
    proform_performance_400i = settings.value(QZSettings::proform_performance_400i, QZSettings::default_proform_performance_400i).toBool();
    proform_treadmill_c700 = settings.value(QZSettings::proform_treadmill_c700, QZSettings::default_proform_treadmill_c700).toBool();
    proform_treadmill_c960i = settings.value(QZSettings::proform_treadmill_c960i, QZSettings::default_proform_treadmill_c960i).toBool();
    nordictrack_tseries5_treadmill = settings.value(QZSettings::nordictrack_tseries5_treadmill, QZSettings::default_nordictrack_tseries5_treadmill).toBool();
    proform_carbon_tl_PFTL59722c = settings.value(QZSettings::proform_carbon_tl_PFTL59722c, QZSettings::default_proform_carbon_tl_PFTL59722c).toBool();
    proform_treadmill_1500_pro = settings.value(QZSettings::proform_treadmill_1500_pro, QZSettings::default_proform_treadmill_1500_pro).toBool();
    proform_505_cst_80_44 = settings.value(QZSettings::proform_505_cst_80_44, QZSettings::default_proform_505_cst_80_44).toBool();
    proform_trainer_8_0 = settings.value(QZSettings::proform_trainer_8_0, QZSettings::default_proform_trainer_8_0).toBool();
    proform_treadmill_705_cst_V80_44 = settings.value(QZSettings::proform_treadmill_705_cst_V80_44, QZSettings::default_proform_treadmill_705_cst_V80_44).toBool();
    nordictrack_t65s_treadmill_81_miles = settings.value(QZSettings::nordictrack_t65s_treadmill_81_miles, QZSettings::default_nordictrack_t65s_treadmill).toBool();
    nordictrack_elite_800 = settings.value(QZSettings::nordictrack_elite_800, QZSettings::default_nordictrack_elite_800).toBool();
    nordictrack_treadmill_ultra_le = settings.value(QZSettings::nordictrack_treadmill_ultra_le, QZSettings::default_nordictrack_treadmill_ultra_le).toBool();
    proform_treadmill_carbon_tls = settings.value(QZSettings::proform_treadmill_carbon_tls, QZSettings::default_proform_treadmill_carbon_tls).toBool();
    proform_treadmill_995i = settings.value(QZSettings::proform_treadmill_995i, QZSettings::default_proform_treadmill_995i).toBool();
    nordictrack_series_7 = settings.value(QZSettings::nordictrack_series_7, QZSettings::default_nordictrack_series_7).toBool();
    proform_treadmill_sport_3_0 = settings.value(QZSettings::proform_treadmill_sport_3_0, QZSettings::default_proform_treadmill_sport_3_0).toBool();

    if (proform_treadmill_995i) {
        // ProForm 995i initialization frames from pkt4658 to pkt4756 (all 25 frames)
        uint8_t initData1[4] = {0xfe, 0x02, 0x08, 0x02}; // pkt4658
        uint8_t initData2[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04,
                                 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00}; // pkt4661
        uint8_t initData3[4] = {0xfe, 0x02, 0x08, 0x02}; // pkt4667
        uint8_t initData4[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04,
                                 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00}; // pkt4670
        uint8_t initData5[4] = {0xfe, 0x02, 0x08, 0x02}; // pkt4676
        uint8_t initData6[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04,
                                 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00}; // pkt4679
        uint8_t initData7[4] = {0xfe, 0x02, 0x0a, 0x02}; // pkt4684
        uint8_t initData8[20] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06,
                                 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00}; // pkt4687
        uint8_t initData9[4] = {0xfe, 0x02, 0x0a, 0x02}; // pkt4694
        uint8_t initData10[20] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06,
                                  0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4697
        uint8_t initData11[4] = {0xfe, 0x02, 0x08, 0x02}; // pkt4703
        uint8_t initData12[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04,
                                  0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4706
        uint8_t initData13[4] = {0xfe, 0x02, 0x2c, 0x04}; // pkt4712
        uint8_t initData14[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28,
                                  0x90, 0x07, 0x01, 0xc8, 0x6c, 0x02, 0xae, 0x4c,
                                  0xf8, 0x9e, 0x4a, 0xe0}; // pkt4715
        uint8_t initData15[20] = {0x01, 0x12, 0x94, 0x4a, 0xf6, 0xa4, 0x70, 0x06,
                                  0xc2, 0x98, 0x5c, 0x12, 0xde, 0x9c, 0x48, 0x0e,
                                  0xfa, 0xb0, 0x64, 0x5a}; // pkt4718
        uint8_t initData16[20] = {0xff, 0x08, 0x06, 0xf4, 0xe0, 0x98, 0x02, 0x00,
                                  0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4721
        uint8_t initData17[4] = {0xfe, 0x02, 0x19, 0x03}; // pkt4726
        uint8_t initData18[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15,
                                  0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c,
                                  0x48, 0x00, 0x00, 0xe0}; // pkt4729
        uint8_t initData19[20] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08,
                                  0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4732
        uint8_t initData20[4] = {0xfe, 0x02, 0x17, 0x03}; // pkt4739
        uint8_t initData21[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13,
                                  0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4742
        uint8_t initData22[20] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4745
        uint8_t initData23[4] = {0xfe, 0x02, 0x19, 0x03}; // pkt4750
        uint8_t initData24[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15,
                                  0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4753
        uint8_t initData25[20] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00,
                                  0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}; // pkt4756

        int sleepms = 400;

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init pkt4658"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init pkt4661"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init pkt4667"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init pkt4670"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init pkt4676"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init pkt4679"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init pkt4684"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init pkt4687"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init pkt4694"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init pkt4697"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init pkt4703"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init pkt4706"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init pkt4712"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init pkt4715"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init pkt4718"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init pkt4721"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init pkt4726"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init pkt4729"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init pkt4732"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init pkt4739"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init pkt4742"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init pkt4745"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init pkt4750"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init pkt4753"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init pkt4756"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_series_7) {
        // NordicTrack Series 7 initialization frames (36 packets: pkt526 to pkt709)
        uint8_t initData1[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[4] = {0xfe, 0x02, 0x0b, 0x02};
        uint8_t initData8[20] = {0xff, 0x0b, 0x02, 0x04, 0x02, 0x07, 0x02, 0x07, 0x82, 0x00, 0x00, 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[4] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[20] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[4] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[20] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[4] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0xe6, 0xda, 0xd4, 0xcc, 0xc2, 0xc6, 0xc8, 0xc8, 0xce, 0xc2};
        uint8_t initData15[20] = {0x01, 0x12, 0xdc, 0xd4, 0xea, 0xfe, 0x10, 0x00, 0x36, 0x2a, 0x44, 0x7c, 0x92, 0xb6, 0xd8, 0xf8, 0x1e, 0x32, 0x4c, 0x64};
        uint8_t initData16[20] = {0xff, 0x08, 0xba, 0xce, 0x20, 0xa0, 0x02, 0x00, 0x00, 0xf5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData17[4] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData18[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData19[20] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData20[4] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData21[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData22[20] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData23[4] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData24[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t initData25[20] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData26[4] = {0xfe, 0x02, 0x14, 0x03};
        uint8_t initData27[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00, 0x0a, 0x13, 0x94, 0x33, 0x00, 0x10, 0x40, 0x10, 0x00, 0x80};
        uint8_t initData28[20] = {0xff, 0x02, 0x18, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData29[4] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t initData30[20] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x04, 0x0c, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02, 0x78, 0x1e, 0x00, 0xae, 0x00, 0x00};
        uint8_t initData31[4] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData32[20] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData33[20] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData34[4] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t initData35[20] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x04, 0x0c, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x28, 0x00, 0x00};
        uint8_t initData36[4] = {0xfe, 0x02, 0x19, 0x03};

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
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData26, sizeof(initData26), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData27, sizeof(initData27), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData28, sizeof(initData28), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData29, sizeof(initData29), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData30, sizeof(initData30), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData31, sizeof(initData31), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData32, sizeof(initData32), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData33, sizeof(initData33), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData34, sizeof(initData34), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData35, sizeof(initData35), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData36, sizeof(initData36), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack10) {
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
    } else if (proform_505_cst_80_44) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                                0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04,
                                0x00, 0x36, 0xa4, 0x08, 0x7a, 0xea, 0x68, 0xdc, 0x46, 0xce};
        uint8_t initData15[] = {0x01, 0x12, 0x4c, 0xb0, 0x32, 0xb2, 0x50, 0xd4, 0x5e, 0xc6,
                                0x74, 0xf8, 0x6a, 0x1a, 0xb8, 0x2c, 0xd6, 0x7e, 0x1c, 0x80};
        uint8_t initData16[] = {0xff, 0x08, 0x22, 0xc2, 0xa0, 0x80, 0x02, 0x00, 0x00, 0x50,
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
        uint8_t noOpData7[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData8[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData10[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData11[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData12[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData12, sizeof(noOpData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_carbon_tl) {
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
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x98, 0xe9, 0x38, 0x95, 0xe0, 0x39, 0x90, 0xfd, 0x58, 0xb9};
        uint8_t initData11[] = {0x01, 0x12, 0x18, 0x75, 0xd0, 0x49, 0xa0, 0x0d, 0x98, 0xe9, 0x78, 0xf5, 0x40, 0xd9, 0x50, 0xdd, 0x58, 0xd9, 0x58, 0xd5};
        uint8_t initData12[] = {0xff, 0x08, 0x50, 0xc9, 0x40, 0xa0, 0x02, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
    } else if (proform_treadmill_1500_pro) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                            0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                                0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0x57, 0x70, 0x93, 0xbc, 0xe7, 0x00, 0x3b, 0x54, 0x87};
        uint8_t initData15[] = {0x01, 0x12, 0xb0, 0xe3, 0x2c, 0x57, 0x80, 0xdb, 0x14, 0x57,
                                0x90, 0xd3, 0x1c, 0x47, 0x80, 0xfb, 0x34, 0x67, 0xd0, 0x03};
        uint8_t initData16[] = {0xff, 0x08, 0x6c, 0xd7, 0x00, 0x90, 0x02, 0x00, 0x00, 0x37,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData17[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData19[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData20[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData22[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData23[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t initData25[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                                

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
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_tseries5_treadmill) {
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
                                0x01, 0x8d, 0x68, 0x49, 0x28, 0x15, 0xf0, 0xe9, 0xc0, 0xbd};
        uint8_t initData11[] = {0x01, 0x12, 0xa8, 0x99, 0x88, 0x75, 0x60, 0x79, 0x70, 0x4d,
                                0x48, 0x49, 0x48, 0x75, 0x70, 0x69, 0x60, 0x9d, 0x88, 0xb9};
        uint8_t initData12[] = {0xff, 0x08, 0xa8, 0xd5, 0xc0, 0xa0, 0x02, 0x00, 0x00, 0xad,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        
        // Extra initialization sequence
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
    } else if (proform_treadmill_c960i) {
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
                                0x01, 0x93, 0xc0, 0xff, 0x34, 0x6b, 0xa0, 0xef, 0x2c, 0x63};
        uint8_t initData11[] = {0x01, 0x12, 0xa0, 0xff, 0x34, 0x8b, 0xc0, 0x0f, 0x7c, 0xd3,
                                0x00, 0x7f, 0xd4, 0x0b, 0x60, 0xcf, 0x2c, 0x83, 0xe0, 0x7f};
        uint8_t initData12[] = {0xff, 0x08, 0xd4, 0x2b, 0x80, 0x90, 0x02, 0x00, 0x00, 0xfb,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        // Additional 9 packets
        uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0xfe, 0x02, 0x14, 0x03};
        uint8_t noOpData8[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00,
                               0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData9[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
        // Additional 9 packets
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_performance_300i) {
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
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0xfd, 0xa8, 0x59, 0x08, 0xc5, 0x70, 0x39, 0xe0, 0xad};
        uint8_t initData15[] = {0x01, 0x12, 0x68, 0x29, 0xe8, 0xa5, 0x60, 0x29, 0x10, 0xdd, 0x88, 0x79, 0x28, 0x05, 0xf0, 0xd9, 0x80, 0x6d, 0x48, 0x29};
        uint8_t initData16[] = {0xff, 0x08, 0x08, 0xe5, 0xc0, 0x80, 0x02, 0x00, 0x00, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData17[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t initData19[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData20[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData22[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData23[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData25[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData26[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData27[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t initData28[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData29[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData30[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData31[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData26, sizeof(initData26), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData27, sizeof(initData27), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData28, sizeof(initData28), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData29, sizeof(initData29), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData30, sizeof(initData30), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData31, sizeof(initData31), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_performance_400i) {
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
            uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0x5d, 0x28, 0xf9, 0xc8, 0xa5, 0x70, 0x59, 0x20, 0x0d};
            uint8_t initData15[] = {0x01, 0x12, 0xe8, 0xc9, 0xa8, 0x85, 0x60, 0x49, 0x50, 0x3d, 0x08, 0x19, 0xe8, 0xe5, 0xf0, 0xf9, 0xc0, 0xcd, 0xc8, 0xc9};
            uint8_t initData16[] = {0xff, 0x08, 0xc8, 0xc5, 0xc0, 0x80, 0x02, 0x00, 0x00, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData20[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData22[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData23[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
            uint8_t initData25[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData26[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t initData27[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t initData28[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData29[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t initData30[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t initData31[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
            writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData26, sizeof(initData26), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData27, sizeof(initData27), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData28, sizeof(initData28), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData29, sizeof(initData29), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData30, sizeof(initData30), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(initData31, sizeof(initData31), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
    } else if (proform_treadmill_c700) {
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
                            0x00, 0x19, 0x78, 0xe5, 0x40, 0xa9, 0x10, 0x8d, 0xf8, 0x69};
        uint8_t initData11[] = {0x01, 0x12, 0xd8, 0x45, 0xb0, 0x39, 0xa0, 0x3d, 0xb8, 0x39,
                            0xb8, 0x25, 0xa0, 0x29, 0xd0, 0x4d, 0xf8, 0x69, 0x18, 0x85};
        uint8_t initData12[] = {0xff, 0x08, 0x30, 0xd9, 0x40, 0x80, 0x02, 0x00, 0x00, 0x7d,
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
        uint8_t noOpData7[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData8[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                            0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData9[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData10[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData11[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData12[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};                            

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
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData12, sizeof(noOpData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);        
} else if (proform_carbon_tl_PFTL59720) {
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
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0xdd, 0x28, 0x79, 0xc8, 0x25, 0x70, 0xd9, 0x20, 0x8d};
        uint8_t initData11[] = {0x01, 0x12, 0xe8, 0x49, 0xa8, 0x05, 0x60, 0xc9, 0x50, 0xbd, 0x08, 0x99, 0xe8, 0x65, 0xf0, 0x79, 0xc0, 0x4d, 0xc8, 0x49};
        uint8_t initData12[] = {0xff, 0x08, 0xc8, 0x45, 0xc0, 0x98, 0x02, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
    } else if (proform_treadmill_sport_70) {
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
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0x31, 0x98, 0x0d, 0x70, 0xe1, 0x50, 0xc5, 0x48, 0xb1};
        uint8_t initData11[] = {0x01, 0x12, 0x38, 0xbd, 0x20, 0xa1, 0x20, 0xa5, 0x28, 0xd1, 0x58, 0xcd, 0x70, 0xe1, 0x90, 0x05, 0xa8, 0x51, 0xf8, 0x9d};
        uint8_t initData12[] = {0xff, 0x08, 0x00, 0xa1, 0x40, 0x80, 0x02, 0x00, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
    } else if (proform_treadmill_575i) {
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

        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0x83, 0x00, 0x8f, 0x14, 0x9b, 0x20, 0xbf, 0x4c, 0xd3};
        uint8_t initData11[] = {0x01, 0x12, 0x60, 0x0f, 0x94, 0x3b, 0xc0, 0x7f, 0x1c, 0xa3, 0x40, 0xef, 0xb4, 0x5b, 0xe0, 0xbf, 0x4c, 0x13, 0xa0, 0x6f};
        uint8_t initData12[] = {0xff, 0x08, 0x34, 0xfb, 0x80, 0x88, 0x02, 0x00, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData8[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_8_7) {
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


        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x1a, 0xa4, 0x3c, 0xce, 0x66, 0xf0, 0x98, 0x22, 0xc2, 0x6c};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);


        uint8_t initData15[] = {0x01, 0x12, 0x04, 0xa6, 0x5e, 0xf8, 0xb0, 0x5a, 0xea, 0xb4, 0x4c, 0x1e, 0xd6, 0x60, 0x28, 0xf2, 0xb2, 0x7c, 0x34, 0xf6};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);


        uint8_t initData16[] = {0xff, 0x08, 0xae, 0x68, 0x60, 0xa0, 0x02, 0x00, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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


        uint8_t initData23[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);


        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);


        uint8_t initData25[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

    } else if (proform_595i_proshox2) {
        uint8_t initData1[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData3[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData4[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData5[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData6[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData8[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData9[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData10[] = {0xfe, 0x02, 0x2c, 0x04};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData11[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData12[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0x36, 0xa4, 0x08, 0x7a, 0xea, 0x68, 0xdc, 0x46, 0xce};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData13[] = {0x01, 0x12, 0x4c, 0xb0, 0x32, 0xb2, 0x50, 0xd4, 0x5e, 0xc6, 0x74, 0xf8, 0x6a, 0x1a, 0xb8, 0x2c, 0xd6, 0x7e, 0x1c, 0x80};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData14[] = {0xff, 0x08, 0x22, 0xc2, 0xa0, 0x80, 0x02, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData15[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData16[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData17[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData18[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData19[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x92, 0x9a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData20[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData21[] = {0xfe, 0x02, 0x0d, 0x02};
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData22[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x04, 0x09, 0x02, 0x01, 0x01, 0x64, 0x00, 0x00, 0x75, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_proshox2) {
        uint8_t initData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x80, 0x0a, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x80, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData8[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0x36, 0xa4, 0x08, 0x7a, 0xea, 0x68, 0xdc, 0x46, 0xce};
        uint8_t initData9[] = {0x01, 0x12, 0x4c, 0xb0, 0x32, 0xb2, 0x50, 0xd4, 0x5e, 0xc6, 0x74, 0xf8, 0x6a, 0x1a, 0xb8, 0x2c, 0xd6, 0x7e, 0x1c, 0x80};
        uint8_t initData10[] = {0xff, 0x08, 0x22, 0xc2, 0xa0, 0x80, 0x02, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
    } else if (proform_treadmill_705_cst_V78_239) {
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

        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData19[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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

        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData25[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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
    } else if (proform_treadmill_705_cst_V80_44) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04, 0x00, 0x41, 0x58, 0x7d, 0x90, 0xb1, 0xd0, 0xf5, 0x28, 0x41};
        uint8_t initData11[] = {0x01, 0x12, 0x78, 0xad, 0xc0, 0xf1, 0x20, 0x75, 0x88, 0xc1, 0x18, 0x5d, 0x90, 0xd1, 0x10, 0x55, 0x88, 0xc1, 0x38, 0x6d};
        uint8_t initData12[] = {0xff, 0x08, 0xa0, 0x11, 0x40, 0x80, 0x02, 0x00, 0x00, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t initData15[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData16[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData17[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData18[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData19[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData20[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData21[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData22[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData23[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t initData24[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_treadmill_exp_5i) {
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
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0xe0, 0xd5, 0xc8, 0xc1, 0xb8, 0xbd, 0xb0, 0xb1, 0xb0, 0xb5};
        uint8_t initData11[] = {0x01, 0x12, 0xc8, 0xc1, 0xd8, 0xed, 0xe0, 0x11, 0x00, 0x35, 0x28, 0x41, 0x78, 0x9d, 0xb0, 0xd1, 0xf0, 0x15, 0x28, 0x41};
        uint8_t initData12[] = {0xff, 0x08, 0x98, 0xad, 0xc0, 0xa0, 0x02, 0x00, 0x00, 0xc4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData8[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
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
    } else if (proform_trainer_8_0) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                                0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x04,
                                0x00, 0x28, 0xec, 0xa2, 0x6e, 0x2c, 0xf8, 0xbe, 0x8a, 0x40};
        uint8_t initData15[] = {0x01, 0x12, 0x14, 0xea, 0xb6, 0x84, 0x70, 0x26, 0x02, 0xf8,
                                0xdc, 0xb2, 0x9e, 0x7c, 0x48, 0x2e, 0x3a, 0x10, 0xe4, 0xfa};
        uint8_t initData16[] = {0xff, 0x08, 0xc6, 0xd4, 0xe0, 0x80, 0x02, 0x00, 0x00, 0x9a,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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

        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                               0x0d, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x62, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00,
                                0x0d, 0x1b, 0x94, 0x31, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData11[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0x2f, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData12[] = {0xfe, 0x02, 0x17, 0x03};

        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData12, sizeof(noOpData12), QStringLiteral("init"), false, false);
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
    } else if (nordictrack_elite_800) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 
                                0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 
                                0x01, 0xff, 0x50, 0xab, 0x0c, 0x6f, 0xc0, 0x23, 0x84, 0xff};
        uint8_t initData15[] = {0x01, 0x12, 0x50, 0xcb, 0x3c, 0xaf, 0x00, 0x93, 0xe4, 0x7f, 
                                0xf0, 0x4b, 0xcc, 0x4f, 0xc0, 0x43, 0xc4, 0x7f, 0xf0, 0x6b};
        uint8_t initData16[] = {0xff, 0x08, 0x1c, 0x8f, 0x00, 0xa0, 0x02, 0x00, 0x00, 0xdf, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData17[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData19[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData20[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData22[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData23[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 
                                0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t initData25[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData26[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t initData27[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 
                                0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
        uint8_t initData28[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x05, 0xed, 0x00, 0x00, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData29[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t initData30[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 
                                0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData31[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        /*uint8_t initData32[] = {0xfe, 0x02, 0x0d, 0x02};
        uint8_t initData33[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x04, 0x09, 0x02, 0x02, 
                                0x00, 0x10, 0x04, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData34[] = {0xfe, 0x02, 0x0f, 0x02};
        uint8_t initData35[] = {0xff, 0x0f, 0x02, 0x04, 0x02, 0x0b, 0x04, 0x0b, 0x02, 0x02, 
                                0x02, 0x10, 0x00, 0x00, 0x01, 0x00, 0x26, 0x00, 0x00, 0x00};*/
    
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
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData26, sizeof(initData26), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData27, sizeof(initData27), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData28, sizeof(initData28), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData29, sizeof(initData29), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData30, sizeof(initData30), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData31, sizeof(initData31), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        /*writeCharacteristic(initData32, sizeof(initData32), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData33, sizeof(initData33), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData34, sizeof(initData34), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData35, sizeof(initData35), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);   */     
     } else if (nordictrack_t65s_treadmill_81_miles) {
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
            uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0xce, 0xc4, 0xb0, 0xaa, 0xa2, 0xa8, 0x94, 0x96, 0x96};
            uint8_t initData15[] = {0x01, 0x12, 0xac, 0xa8, 0xa2, 0xba, 0xd0, 0xdc, 0xce, 0xfe, 0x14, 0x00, 0x3a, 0x52, 0x78, 0x64, 0x86, 0xa6, 0xfc, 0x18};
            uint8_t initData16[] = {0xff, 0x08, 0x32, 0x4a, 0xa0, 0x88, 0x02, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData7[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData8[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
            uint8_t noOpData9[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData10[] = {0xfe, 0x02, 0x14, 0x03};
            uint8_t noOpData11[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00, 0x0a, 0x1b, 0x94, 0x30, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
            uint8_t noOpData12[] = {0xff, 0x02, 0x18, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData13[] = {0xfe, 0x02, 0x13, 0x03};
            uint8_t noOpData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x0f, 0x04, 0x0f, 0x02, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20};
            uint8_t noOpData15[] = {0xff, 0x01, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData16[] = {0xfe, 0x02, 0x13, 0x03};
            uint8_t noOpData17[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x0f, 0x04, 0x0f, 0x02, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40};
            uint8_t noOpData18[] = {0xff, 0x01, 0x5e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
            writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData12, sizeof(noOpData12), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData13, sizeof(noOpData13), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData14, sizeof(noOpData14), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData15, sizeof(noOpData15), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData16, sizeof(noOpData16), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData17, sizeof(noOpData17), QStringLiteral("init"), false, false);
            QThread::msleep(sleepms);
            writeCharacteristic(noOpData18, sizeof(noOpData18), QStringLiteral("init"), false, false);
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
    } else if (proform_carbon_tl_PFTL59722c) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0xe1, 0xd8, 0xdd, 0xd0, 0xd1, 0xd0, 0xd5, 0xe8, 0xe1};
        uint8_t initData11[] = {0x01, 0x12, 0xf8, 0x0d, 0x00, 0x11, 0x20, 0x55, 0x48, 0x61, 0x98, 0xbd, 0xd0, 0xf1, 0x10, 0x35, 0x48, 0x61, 0xb8, 0xcd};
        uint8_t initData12[] = {0xff, 0x08, 0xe0, 0x31, 0x40, 0x98, 0x02, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData8[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t noOpData9[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData10[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData11[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
        uint8_t noOpData12[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x01, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData13[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData15[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData13, sizeof(noOpData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData12, sizeof(noOpData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData14, sizeof(noOpData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(noOpData15, sizeof(noOpData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_s20i_treadmill) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};  // from pkt1268
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1286
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};  // from pkt1319
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1329
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};  // from pkt1362
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1370
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};  // from pkt1431
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1436
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};  // from pkt1445
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                                0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1447
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};  // from pkt1454
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1456
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};  // from pkt1466
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x01, 0x85, 0x08, 0x91, 0x18, 0xad, 0x30, 0xc1, 0x50, 0xe5};  // from pkt1468
        uint8_t initData15[] = {0x01, 0x12, 0x88, 0x11, 0xb8, 0x5d, 0xe0, 0x81, 0x20, 0xc5,
                                0x68, 0x31, 0xd8, 0x6d, 0x30, 0xc1, 0x90, 0x25, 0xe8, 0xb1};

        uint8_t initData16[] = {0xff, 0x08, 0x78, 0x3d, 0xc0, 0x98, 0x02, 0x00, 0x00, 0xed,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1475
        uint8_t initData17[] = {0xfe, 0x02, 0x17, 0x03};  // from pkt1482
        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1484
        uint8_t initData19[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1486
        uint8_t initData20[] = {0xfe, 0x02, 0x19, 0x03};  // from pkt1493
        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1495
        uint8_t initData22[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1498
        uint8_t initData23[] = {0xfe, 0x02, 0x19, 0x03};  // from pkt1506
        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};  // from pkt1509
        uint8_t initData25[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};  // from pkt1511

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
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (proform_treadmill_carbon_tls) {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 
                                0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 
                                0x6e, 0xa6, 0xe4, 0x18, 0x5a, 0x9a, 0xe8, 0x2c, 0x66, 0xbe};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData15[] = {0x01, 0x12, 0x0c, 0x40, 0x92, 0xe2, 0x50, 0x84, 0xfe, 0x56, 
                                0xb4, 0xe8, 0x4a, 0xaa, 0x38, 0x9c, 0xf6, 0x4e, 0xdc, 0x30};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData16[] = {0xff, 0x08, 0x82, 0x12, 0xa0, 0xa0, 0x02, 0x00, 0x00, 0x71, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData20[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData22[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData23[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 
                                0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData25[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData26[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData26, sizeof(initData26), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData27[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x00, 
                                0x0d, 0x13, 0x96, 0x31, 0x00, 0x00, 0x40, 0x10, 0x00, 0x80};
        writeCharacteristic(initData27, sizeof(initData27), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData28[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x05, 0xed, 0x00, 0x00, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData28, sizeof(initData28), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData29[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData29, sizeof(initData29), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData30[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 
                                0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData30, sizeof(initData30), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);

        uint8_t initData31[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x80, 0x00, 0x10, 0x82, 0x00, 
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData31, sizeof(initData31), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
    } else if (nordictrack_treadmill_ultra_le) {
        // NordicTrack Treadmill Ultra LE initialization sequence
        // Frames extracted from ultrale.c with first 11 bytes removed

        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02}; // Frame from pkt5021
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5032
        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02}; // Frame from pkt5048
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5059
        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02}; // Frame from pkt5071
        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5077
        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02}; // Frame from pkt5098
        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5106
        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02}; // Frame from pkt5119
        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                                0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5132
        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02}; // Frame from pkt5142
        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5149
        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04}; // Frame from pkt5166
        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07,
                                0x8e, 0xd6, 0x24, 0x68, 0xba, 0x0a, 0x68, 0xbc, 0x06, 0x6e}; // Frame from pkt5171
        uint8_t initData15[] = {0x01, 0x12, 0xcc, 0x10, 0x72, 0xd2, 0x50, 0xb4, 0x1e, 0x66,
                                0xf4, 0x58, 0xaa, 0x3a, 0xb8, 0x0c, 0x96, 0x1e, 0x9c, 0xe0}; // Frame from pkt5176
        uint8_t initData16[] = {0xff, 0x08, 0x62, 0xe2, 0xa0, 0xa0, 0x02, 0x00, 0x00, 0x61,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5182
        uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03}; // Frame from pkt5211
        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5219
        uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5225
        uint8_t initData20[] = {0xfe, 0x02, 0x17, 0x03}; // Frame from pkt5249
        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5255
        uint8_t initData22[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5261
        uint8_t initData23[] = {0xfe, 0x02, 0x19, 0x03}; // Frame from pkt5267
        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00,
                                0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0}; // Frame from pkt5274
        uint8_t initData25[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // Frame from pkt5281


               // Send initialization sequence for NordicTrack Treadmill Ultra LE
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
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(sleepms);
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
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
    } else if (proform_treadmill_sport_3_0) {
        // Init sequence (39 packets)
        uint8_t sport30_init_001[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t sport30_init_002[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_003[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t sport30_init_004[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x80, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_005[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t sport30_init_006[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x04, 0x04, 0x88, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_007[] = {0xfe, 0x02, 0x0b, 0x02};
        uint8_t sport30_init_008[] = {0xff, 0x0b, 0x02, 0x04, 0x02, 0x07, 0x02, 0x07, 0x82, 0x00, 0x00, 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_009[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t sport30_init_010[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_011[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t sport30_init_012[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_013[] = {0xfe, 0x02, 0x2c, 0x04};
        uint8_t sport30_init_014[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x04, 0x28, 0x90, 0x07, 0x01, 0x56, 0x24, 0xe8, 0xba, 0x8a, 0x68, 0x3c, 0x06, 0xee};
        uint8_t sport30_init_015[] = {0x01, 0x12, 0xcc, 0x90, 0x72, 0x52, 0x50, 0x34, 0x1e, 0xe6, 0xf4, 0xd8, 0xaa, 0xba, 0xb8, 0x8c, 0x96, 0x9e, 0x9c, 0x60};
        uint8_t sport30_init_016[] = {0xff, 0x08, 0x62, 0x62, 0xa0, 0x98, 0x02, 0x00, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_017[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t sport30_init_018[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_019[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_020[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t sport30_init_021[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x04, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_022[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_023[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t sport30_init_024[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x00, 0x10, 0x00, 0xd8, 0x1c, 0x48, 0x00, 0x00, 0xe0};
        uint8_t sport30_init_025[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_026[] = {0xfe, 0x02, 0x14, 0x03};
        uint8_t sport30_init_027[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x04, 0x10, 0x02, 0x00, 0x0a, 0x13, 0x94, 0x33, 0x00, 0x10, 0x40, 0x10, 0x00, 0x80};
        uint8_t sport30_init_028[] = {0xff, 0x02, 0x18, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_029[] = {0xfe, 0x02, 0x11, 0x02};
        uint8_t sport30_init_030[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x04, 0x0d, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x08, 0x58, 0x02, 0x00, 0x7a, 0x00};
        uint8_t sport30_init_031[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t sport30_init_032[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_033[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_034[] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t sport30_init_035[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x04, 0x0c, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02, 0xe4, 0x1f, 0x00, 0x1b, 0x00, 0x00};
        uint8_t sport30_init_036[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t sport30_init_037[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x04, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_037a[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x81, 0x00, 0x10, 0x7d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t sport30_init_038[] = {0xfe, 0x02, 0x10, 0x02};
        uint8_t sport30_init_039[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x04, 0x0c, 0x02, 0x05, 0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x28, 0x00, 0x00};

        int sleepms = 400;
        writeCharacteristic(sport30_init_001, sizeof(sport30_init_001), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_002, sizeof(sport30_init_002), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_003, sizeof(sport30_init_003), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_004, sizeof(sport30_init_004), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_005, sizeof(sport30_init_005), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_006, sizeof(sport30_init_006), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_007, sizeof(sport30_init_007), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_008, sizeof(sport30_init_008), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_009, sizeof(sport30_init_009), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_010, sizeof(sport30_init_010), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_011, sizeof(sport30_init_011), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_012, sizeof(sport30_init_012), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_013, sizeof(sport30_init_013), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_014, sizeof(sport30_init_014), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_015, sizeof(sport30_init_015), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_016, sizeof(sport30_init_016), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_017, sizeof(sport30_init_017), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_018, sizeof(sport30_init_018), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_019, sizeof(sport30_init_019), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_020, sizeof(sport30_init_020), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_021, sizeof(sport30_init_021), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_022, sizeof(sport30_init_022), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_023, sizeof(sport30_init_023), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_024, sizeof(sport30_init_024), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_025, sizeof(sport30_init_025), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_026, sizeof(sport30_init_026), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_027, sizeof(sport30_init_027), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_028, sizeof(sport30_init_028), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_029, sizeof(sport30_init_029), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_030, sizeof(sport30_init_030), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_031, sizeof(sport30_init_031), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_032, sizeof(sport30_init_032), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_033, sizeof(sport30_init_033), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_034, sizeof(sport30_init_034), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_035, sizeof(sport30_init_035), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_036, sizeof(sport30_init_036), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_037, sizeof(sport30_init_037), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_037a, sizeof(sport30_init_037a), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);        
        writeCharacteristic(sport30_init_038, sizeof(sport30_init_038), QStringLiteral("init"), false, true);
        QThread::msleep(sleepms);
        writeCharacteristic(sport30_init_039, sizeof(sport30_init_039), QStringLiteral("init"), false, true);
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

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
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
                &QLowEnergyService::errorOccurred,
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
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
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
                &QLowEnergyController::errorOccurred,
                this, &proformtreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &proformtreadmill::controllerStateChanged);

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
