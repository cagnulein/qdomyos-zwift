#include "solef80treadmill.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include <chrono>

using namespace std::chrono_literals;

QBluetoothUuid _gattWriteCharCustomService(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));
QBluetoothUuid _gattWriteCharControlPointId(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
QBluetoothUuid _gattNotifyCharId(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

solef80treadmill::solef80treadmill(bool noWriteResistance, bool noHeartService) {
#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif

    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &solef80treadmill::update);
    refresh->start(300ms);
}

void solef80treadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    QSettings settings;
    bool inclination =
        settings.value(QZSettings::sole_treadmill_inclination, QZSettings::default_sole_treadmill_inclination).toBool();

    if (!inclination) {
        qDebug() << "inclination support disabled";
        return;
    }

    if (!gattCustomService) {
        qDebug() << "no gattCustomService available";
        return;
    }

    if (wait_for_response) {
        connect(this, &solef80treadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(2000, &loop, SLOT(quit()));
    } else {
        connect(gattCustomService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(2000, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharCustomService.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCustomService->writeCharacteristic(gattWriteCharCustomService, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCustomService->writeCharacteristic(gattWriteCharCustomService, *writeBuffer);
    }

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}

void solef80treadmill::btinit() {

    QSettings settings;
    bool f65 = settings.value(QZSettings::sole_treadmill_f65, QZSettings::default_sole_treadmill_f65).toBool();
    bool f63 = settings.value(QZSettings::sole_treadmill_f63, QZSettings::default_sole_treadmill_f63).toBool();
    bool tt8 = settings.value(QZSettings::sole_treadmill_tt8, QZSettings::default_sole_treadmill_tt8).toBool();

    uint8_t initData01[] = {0x5b, 0x01, 0xf0, 0x5d};
    uint8_t initData01a[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
    uint8_t initData02[] = {0x5b, 0x02, 0x03, 0x01, 0x5d};
    uint8_t initData03[] = {0x5b, 0x04, 0x00, 0x09, 0x4f, 0x4b, 0x5d};
    uint8_t initData03_f63[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
    uint8_t initData03b_f63[] = {0x5b, 0x04, 0x00, 0x09, 0x4f, 0x4b, 0x5d};
    uint8_t initData03c_f63[] = {0x5b, 0x02, 0x03, 0x00, 0x5d};
    uint8_t initData03d_f63[] = {0x5b, 0x02, 0xf1, 0x06, 0x5d};
    uint8_t initData03e_f63[] = {0x5b, 0x04, 0x00, 0xf1, 0x4f, 0x4b, 0x5d};
    uint8_t initData03f_f63[] = {0x5b, 0x02, 0x03, 0x01, 0x5d};
    uint8_t initData03_tt8[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
    uint8_t initData04_f63[] = {0x5b, 0x06, 0x07, 0x00, 0x23, 0x00, 0x84, 0x40, 0x5d};
    uint8_t initData04_tt8[] = {0x5b, 0x06, 0x07, 0x01, 0x23, 0x00, 0x9b, 0x43, 0x5d};
    uint8_t initData04[] = {0x5b, 0x06, 0x07, 0x01, 0x23, 0x00, 0x9b, 0xaa, 0x5d};
    uint8_t initData05_f63[] = {0x5b, 0x03, 0x08, 0x30, 0x07, 0x5d};
    uint8_t initData05_tt8[] = {0x5b, 0x03, 0x08, 0x20, 0x02, 0x5d};
    uint8_t initData05[] = {0x5b, 0x03, 0x08, 0x10, 0x01, 0x5d};
    uint8_t initData06[] = {0x5b, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x5d};
    uint8_t initData06_tt8[] = {0x5b, 0x05, 0x04, 0x0a, 0x00, 0x00, 0x00, 0x5d};
    uint8_t initData07[] = {0x5b, 0x02, 0x22, 0x09, 0x5d};
    uint8_t initData08[] = {0x5b, 0x02, 0x02, 0x02, 0x5d};
    uint8_t initData09[] = {0x5b, 0x04, 0x00, 0x40, 0x4f, 0x4b, 0x5d};
    uint8_t initData10[] = {0x5b, 0x02, 0x03, 0x04, 0x5d};

    uint8_t initData07_f63[] = {0x5b, 0x16, 0x24, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData07b_f63[] = {0x00, 0x00, 0x00, 0x00, 0x5d};
    // uint8_t initData10[] = {0x5b, 0x02, 0x03, 0x04, 0x5d};

    if (gattCustomService) {
        writeCharacteristic(initData01, sizeof(initData01), QStringLiteral("init1"), false, true);
        waitForAPacket();

        if (f65) {
            waitForAPacket();
            writeCharacteristic(initData01a, sizeof(initData01a), QStringLiteral("init1a"), false, true);
        }

        writeCharacteristic(initData02, sizeof(initData02), QStringLiteral("init2"), false, true);
        writeCharacteristic(initData02, sizeof(initData02), QStringLiteral("init2"), false, true);
        writeCharacteristic(initData02, sizeof(initData02), QStringLiteral("init2"), false, true);
        writeCharacteristic(initData02, sizeof(initData02), QStringLiteral("init2"), false, true);

        if (!f65)
            writeCharacteristic(initData03, sizeof(initData03), QStringLiteral("init3"), false, true);

        if (tt8) {
            writeCharacteristic(initData03_tt8, sizeof(initData03_tt8), QStringLiteral("init3_tt8"), false, false);
            writeCharacteristic(initData03_tt8, sizeof(initData03_tt8), QStringLiteral("init3_tt8"), false, false);
            writeCharacteristic(initData03_tt8, sizeof(initData03_tt8), QStringLiteral("init3_tt8"), false, false);
            writeCharacteristic(initData03_tt8, sizeof(initData03_tt8), QStringLiteral("init3_tt8"), false, true);

            writeCharacteristic(initData04_tt8, sizeof(initData04_tt8), QStringLiteral("init4_tt8"), false, true);
            writeCharacteristic(initData05_tt8, sizeof(initData05_tt8), QStringLiteral("init5_tt8"), false, false);
            writeCharacteristic(initData05_tt8, sizeof(initData05_tt8), QStringLiteral("init5_tt8"), false, true);
            writeCharacteristic(initData05_tt8, sizeof(initData05_tt8), QStringLiteral("init5_tt8"), false, false);
            writeCharacteristic(initData05_tt8, sizeof(initData05_tt8), QStringLiteral("init5_tt8"), false, true);

            writeCharacteristic(initData06_tt8, sizeof(initData06_tt8), QStringLiteral("init6_tt8"), false, false);
            writeCharacteristic(initData06_tt8, sizeof(initData06_tt8), QStringLiteral("init6_tt8"), false, true);

            writeCharacteristic(initData07, sizeof(initData07), QStringLiteral("init7"), false, false);
            writeCharacteristic(initData07, sizeof(initData07), QStringLiteral("init7"), false, true);

            writeCharacteristic(initData08, sizeof(initData08), QStringLiteral("init8"), false, true);

        } else if (f63) {
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);

            writeCharacteristic(initData03b_f63, sizeof(initData03b_f63), QStringLiteral("init3b_f63"), false, true);

            writeCharacteristic(initData03c_f63, sizeof(initData03c_f63), QStringLiteral("init3c_f63"), false, true);
            writeCharacteristic(initData03c_f63, sizeof(initData03c_f63), QStringLiteral("init3c_f63"), false, true);
            writeCharacteristic(initData03c_f63, sizeof(initData03c_f63), QStringLiteral("init3c_f63"), false, true);

            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, false);

            writeCharacteristic(initData03d_f63, sizeof(initData03d_f63), QStringLiteral("init3d_f63"), false, true);

            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);

            writeCharacteristic(initData03e_f63, sizeof(initData03e_f63), QStringLiteral("init3e_f63"), false, false);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);

            writeCharacteristic(initData03f_f63, sizeof(initData03f_f63), QStringLiteral("init3f_f63"), false, true);
            writeCharacteristic(initData03f_f63, sizeof(initData03f_f63), QStringLiteral("init3f_f63"), false, true);

            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);

            writeCharacteristic(initData04_f63, sizeof(initData04_f63), QStringLiteral("init4_f63"), false, true);

            writeCharacteristic(initData05_f63, sizeof(initData05_f63), QStringLiteral("init5_f63"), false, true);

            writeCharacteristic(initData06, sizeof(initData06), QStringLiteral("init6"), false, true);

            writeCharacteristic(initData07_f63, sizeof(initData07_f63), QStringLiteral("init7a"), false, false);
            writeCharacteristic(initData07b_f63, sizeof(initData07b_f63), QStringLiteral("init7b"), false, false);
            writeCharacteristic(initData07_f63, sizeof(initData07_f63), QStringLiteral("init7a"), false, true);
            writeCharacteristic(initData07b_f63, sizeof(initData07b_f63), QStringLiteral("init7b"), false, false);
            writeCharacteristic(initData07_f63, sizeof(initData07_f63), QStringLiteral("init7a"), false, true);
            writeCharacteristic(initData07b_f63, sizeof(initData07b_f63), QStringLiteral("init7b"), false, false);

            writeCharacteristic(initData07, sizeof(initData07), QStringLiteral("init7"), false, false);
            writeCharacteristic(initData07, sizeof(initData07), QStringLiteral("init7"), false, true);

            writeCharacteristic(initData08, sizeof(initData08), QStringLiteral("init8"), false, true);

            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);

            writeCharacteristic(initData09, sizeof(initData09), QStringLiteral("init9"), false, true);
            writeCharacteristic(initData09, sizeof(initData09), QStringLiteral("init9"), false, true);

            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);
            writeCharacteristic(initData03_f63, sizeof(initData03_f63), QStringLiteral("init3_f63"), false, true);

            writeCharacteristic(initData09, sizeof(initData09), QStringLiteral("init9"), false, true);

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init10"), false, true);
            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init10"), false, true);
            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init10"), false, true);
            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init10"), false, true);

        } else if (f65) {
            writeCharacteristic(initData01a, sizeof(initData01a), QStringLiteral("init1a"), false, false);
            writeCharacteristic(initData02, sizeof(initData02), QStringLiteral("init2"), false, true);
            writeCharacteristic(initData03, sizeof(initData03), QStringLiteral("init3"), false, true);
            writeCharacteristic(initData01a, sizeof(initData01a), QStringLiteral("init1a"), false, true);

            writeCharacteristic(initData04, sizeof(initData04), QStringLiteral("init4"), false, true);
            writeCharacteristic(initData05, sizeof(initData05), QStringLiteral("init5"), false, true);
            writeCharacteristic(initData05, sizeof(initData05), QStringLiteral("init5"), false, true);
            writeCharacteristic(initData06, sizeof(initData06), QStringLiteral("init6"), false, true);
            writeCharacteristic(initData07, sizeof(initData07), QStringLiteral("init7"), false, true);

            writeCharacteristic(initData08, sizeof(initData08), QStringLiteral("init8"), false, true);
            writeCharacteristic(initData08, sizeof(initData08), QStringLiteral("init8"), false, true);
            writeCharacteristic(initData08, sizeof(initData08), QStringLiteral("init8"), false, true);
            writeCharacteristic(initData08, sizeof(initData08), QStringLiteral("init8"), false, true);
            writeCharacteristic(initData01a, sizeof(initData01a), QStringLiteral("init1a"), false, true);
            writeCharacteristic(initData09, sizeof(initData09), QStringLiteral("init9"), false, true);
        }

        // start workout
        /*writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init10"), false, true);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init10"), false, true);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init10"), false, true);*/
    }

    initDone = true;
}

void solef80treadmill::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &solef80treadmill::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(500, &loop, SLOT(quit()));
    loop.exec();
}

void solef80treadmill::update() {

    QSettings settings;
    bool f65 = settings.value(QZSettings::sole_treadmill_f65, QZSettings::default_sole_treadmill_f65).toBool();
    bool f63 = settings.value(QZSettings::sole_treadmill_f63, QZSettings::default_sole_treadmill_f63).toBool();
    bool tt8 = settings.value(QZSettings::sole_treadmill_tt8, QZSettings::default_sole_treadmill_tt8).toBool();

    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    if (initRequest && firstStateChanged) {
        btinit();
        initRequest = false;
    } else if (bluetoothDevice.isValid() && initDone //&&

               // m_control->state() == QLowEnergyController::DiscoveredState //&&
               // gattCommunicationChannelService &&
               // gattWriteCharacteristic.isValid() &&
               // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {

        QSettings settings;
        bool sole_treadmill_inclination_fast =
            settings
                .value(QZSettings::sole_treadmill_inclination_fast, QZSettings::default_sole_treadmill_inclination_fast)
                .toBool();
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        uint8_t noop[] = {0x5b, 0x04, 0x00, 0x10, 0x4f, 0x4b, 0x5d};
        uint8_t noop2[] = {0x5b, 0x04, 0x00, 0x06, 0x4f, 0x4b, 0x5d};

        if (gattCustomService) {
            writeCharacteristic(noop, sizeof(noop), QStringLiteral("noop"), false, true);
            if (f65 || f63 || tt8)
                writeCharacteristic(noop2, sizeof(noop2), QStringLiteral("noop2"), false, true);
        }

        int max_speed_loop = 0;
        if(requestSpeed != -1) {
            max_speed_loop = (fabs(requestSpeed - currentSpeed().value()) * 10.0) - 1;
        }

        do {
            if (requestSpeed != -1) {
                if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                    emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed) + " " + QString::number(max_speed_loop));
                    forceSpeed(requestSpeed);
                }
                // i have to do the reset on when the speed is equal to the current
                // requestSpeed = -1;
            }
        } while (requestSpeed != -1 && sole_treadmill_inclination_fast && max_speed_loop);

        int max_inclination_loop = 0;
        if(requestInclination != -100) {
            max_inclination_loop = abs(requestInclination - (int)currentInclination().value());
        }
        do {
            if (requestInclination != -100) {
                if (requestInclination < 0)
                    requestInclination = 0;
                // this treadmill has only 1% step inclination
                if ((int)requestInclination != (int)currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 15) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination) + " " + QString::number(max_inclination_loop));
                    forceIncline(requestInclination);
                } else if ((int)requestInclination == (int)currentInclination().value()) {
                    qDebug() << "int inclination match the current one" << requestInclination
                             << currentInclination().value();
                    requestInclination = -100;
                }
                // i have to do the reset on when the inclination is equal to the current
                // requestInclination = -100;
            }
        } while (requestInclination != -100 && sole_treadmill_inclination_fast && max_inclination_loop);

        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            uint8_t start[] = {0x5b, 0x02, 0x03, 0x04, 0x5d};
            uint8_t start2[] = {0x5b, 0x04, 0x00, 0x40, 0x4f, 0x4b, 0x5d};

            if (gattCustomService) {
                writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
                writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
                writeCharacteristic(start, sizeof(start), QStringLiteral("start"), false, true);
                if (!f65)
                    writeCharacteristic(start2, sizeof(start2), QStringLiteral("start"), false, true);
            }
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            if(treadmill_type == F63) {
                uint8_t stop[] = {0x5b, 0x02, 0xf1, 0x06, 0x5d};
                uint8_t stop1[] = {0x5b, 0x02, 0x03, 0x06, 0x5d};

                if (gattCustomService) {
                    writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop"), false, true);
                }
            } else {
                uint8_t stop[] = {0x5b, 0x02, 0x03, 0x06, 0x5d};
                uint8_t stop1[] = {0x5b, 0x02, 0x03, 0x07, 0x5d};
                uint8_t stop2[] = {0x5b, 0x04, 0x00, 0x32, 0x4f, 0x4b, 0x5d};

                if (gattCustomService) {
                    writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop, sizeof(stop), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop2, sizeof(stop2), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop"), false, true);
                    writeCharacteristic(stop1, sizeof(stop1), QStringLiteral("stop"), false, true);
                }
            }

            requestStop = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

double solef80treadmill::minStepInclination() { return 1.0; }

void solef80treadmill::forceSpeed(double requestSpeed) {
    uint8_t up[] = {0x5b, 0x02, 0xf1, 0x02, 0x5d};
    uint8_t down[] = {0x5b, 0x02, 0xf1, 0x03, 0x5d};

    if (requestSpeed > Speed.value()) {
        if (requestSpeedState == IDLE)
            requestSpeedState = UP;
        else if (requestSpeedState == DOWN) {
            requestSpeedState = IDLE;
            this->requestSpeed = -1;
            return;
        }

        if (gattCustomService)
            writeCharacteristic(up, sizeof(up), QStringLiteral("speed up"), false, true);
    } else if (requestSpeed < Speed.value()) {
        if (requestSpeedState == IDLE)
            requestSpeedState = DOWN;
        else if (requestSpeedState == UP) {
            requestSpeedState = IDLE;
            this->requestSpeed = -1;
            return;
        }
        if (gattCustomService)
            writeCharacteristic(down, sizeof(down), QStringLiteral("speed down"), false, true);
    } else {
        this->requestSpeed = -1;
        requestSpeedState = IDLE;
    }
}

void solef80treadmill::forceIncline(double requestIncline) {
    uint8_t up[] = {0x5b, 0x02, 0xf1, 0x04, 0x5d};
    uint8_t down[] = {0x5b, 0x02, 0xf1, 0x05, 0x5d};

    if (requestIncline > Inclination.value()) {
        if (requestInclinationState == IDLE)
            requestInclinationState = UP;
        else if (requestInclinationState == DOWN) {
            requestInclinationState = IDLE;
            this->requestInclination = -100;
            return;
        }
        if (gattCustomService)
            writeCharacteristic(up, sizeof(up), QStringLiteral("Inclination up"), false, true);
    } else if (requestIncline < Inclination.value()) {
        if (requestInclinationState == IDLE)
            requestInclinationState = DOWN;
        else if (requestInclinationState == UP) {
            requestInclinationState = IDLE;
            this->requestInclination = -100;
            return;
        }
        if (gattCustomService)
            writeCharacteristic(down, sizeof(down), QStringLiteral("Inclination down"), false, true);
    } else {
        this->requestInclination = -100;
        requestInclinationState = IDLE;
    }
}

void solef80treadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void solef80treadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    double heart = 0; // NOTE : Should be initialized with a value to shut clang-analyzer's
                      // UndefinedBinaryOperatorResult
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool f65 = settings.value(QZSettings::sole_treadmill_f65, QZSettings::default_sole_treadmill_f65).toBool();
    bool f63 = settings.value(QZSettings::sole_treadmill_f63, QZSettings::default_sole_treadmill_f63).toBool();
    bool tt8 = settings.value(QZSettings::sole_treadmill_tt8, QZSettings::default_sole_treadmill_tt8).toBool();

    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + " " + QString::number(newValue.length()) +
               " " + newValue.toHex(' '));

    if (characteristic.uuid() == _gattNotifyCharId) {
        emit packetReceived();
        // when treadmill is in the inclination mode and it's in pause it doesn't send at all frames
        // so when you resume it, there is a huge time difference and a weird odometer will result
        if (paused) {
            qDebug() << "solef80treadmill inclination mode paused on, resetting timer...";
            Speed = 0;
            lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
        }
    }

    if (characteristic.uuid() == _gattNotifyCharId && newValue.length() == 18) {

        // the treadmill send the speed in miles always
        double miles = 1;
        if (settings.value(QZSettings::sole_treadmill_miles, QZSettings::default_sole_treadmill_miles).toBool())
            miles = 1.60934;

        QDateTime now = QDateTime::currentDateTime();

        Speed = ((double)((uint8_t)newValue.at(10)) / 10.0) * miles;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        Inclination = (double)((uint8_t)newValue.at(11));
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

        if (!disable_hr_frommachinery) {
            heart = (double)((uint8_t)newValue.at(9));
            emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
        }

        Distance += ((Speed.value() / 3600000.0) * ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

        if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                         // kg * 3.5) / 200 ) / 60
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        lastRefreshCharacteristicChanged = now;

    } else if ((characteristic.uuid() == _gattNotifyCharId && newValue.length() == 5 && newValue.at(0) == 0x5b &&
                newValue.at(1) == 0x02 && newValue.at(2) == 0x03) &&
               !f63) {
        // stop event from the treadmill
        qDebug() << "stop/pause event detected from the treadmill";
        initRequest = true;

    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACD)) {
        lastPacket = newValue;

        // default flags for this treadmill is 84 04

        union flags {
            struct {

                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t totalDistance : 1;
                uint16_t inclination : 1;
                uint16_t elevation : 1;
                uint16_t instantPace : 1;
                uint16_t averagePace : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t forceBelt : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        QDateTime now = QDateTime::currentDateTime();
        if (Flags.totalDistance) {
            // ignoring the distance, because it's a total life odometer
            // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
            // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
            index += 3;
        }
        // else
        {
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.inclination) {
            Inclination = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          10.0;
            index += 4; // the ramo value is useless
            emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        }

        if (Flags.elevation) {
            index += 4; // TODO
        }

        if (Flags.instantPace) {
            index += 1; // TODO
        }

        if (Flags.averagePace) {
            index += 1; // TODO
        }

        if (Flags.expEnergy) {
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
            if (watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
                KCal +=
                    ((((0.048 *
                            ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                        1.19) *
                       settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate) {
                if (index < newValue.length()) {

                    heart = ((double)(((uint8_t)newValue.at(index))));
                    emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
                } else {
                    emit debug(QStringLiteral("Error on parsing heart!"));
                }
                // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
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

        if (Flags.forceBelt) {
            // todo
        }

        lastRefreshCharacteristicChanged = now;
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 ||
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool()) {
            update_hr_from_external();
        } else {
            Heart = heart;
        }
    }

    cadenceFromAppleWatch();

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void solef80treadmill::stateChanged(QLowEnergyService::ServiceState state) {

    QSettings settings;
    bool inclination =
        settings.value(QZSettings::sole_treadmill_inclination, QZSettings::default_sole_treadmill_inclination).toBool();

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &solef80treadmill::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &solef80treadmill::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &solef80treadmill::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &solef80treadmill::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &solef80treadmill::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &solef80treadmill::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify &&
                    ((c.uuid() == _gattNotifyCharId && inclination) || (!inclination))) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                    /*} else if ((c.properties() & QLowEnergyCharacteristic::Indicate) ==
                               QLowEnergyCharacteristic::Indicate) {
                        QByteArray descriptor;
                        descriptor.append((char)0x02);
                        descriptor.append((char)0x00);
                        if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                            s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration),
                       descriptor); } else { qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                     << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                     << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                     << QStringLiteral(" is not valid");
                        }

                        qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");*/
                } else if ((c.properties() & QLowEnergyCharacteristic::Read) == QLowEnergyCharacteristic::Read) {
                    // s->readCharacteristic(c);
                    // qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }

                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("Custom service and Control Point found");
                    gattWriteCharCustomService = c;
                    gattCustomService = s;
                }
            }
        }
    }

    // ******************************************* virtual treadmill init *************************************
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
        if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual treadmill interface..."));

            auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
            connect(virtualTreadmill, &virtualtreadmill::debug, this, &solef80treadmill::debug);
            connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                    &solef80treadmill::changeInclinationRequested);
            this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
}

void solef80treadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void solef80treadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void solef80treadmill::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void solef80treadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void solef80treadmill::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
}

void solef80treadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;
    firstStateChanged = 0;
    auto services_list = m_control->services();
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &solef80treadmill::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
    }
}

void solef80treadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("solef80treadmill::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void solef80treadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("solef80treadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void solef80treadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    QSettings settings;
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        if (device.name().toUpper().startsWith(QStringLiteral("F63")))
            treadmill_type = F63;
        else if(device.name().toUpper().startsWith(QStringLiteral("TRX7.5"))) {
            treadmill_type = TRX7_5;
            qDebug() << "TRX7.5 workarkound enabled";
            settings.setValue(QZSettings::sole_treadmill_inclination, true);
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &solef80treadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &solef80treadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &solef80treadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &solef80treadmill::controllerStateChanged);

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

bool solef80treadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void solef80treadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}
