#include "gpiotreadmill.h"
#include "ios/lockscreen.h"
#include "keepawakehelper.h"
#include "virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>

using namespace std::chrono_literals;

// set speed and incline to 0
uint8_t initData1[] = {0xf0, 0xc8, 0x01, 0xb9};
uint8_t initData2[] = {0xf0, 0xc9, 0xb9};

uint8_t noOpData[] = {0xf0, 0xac, 0x9c};

// stop tape
uint8_t initDataF0C800B8[] = {0xf0, 0xc8, 0x00, 0xb8};

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

QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));
QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("49535343-8841-43f4-a8d4-ecbe34729bb3"));
QBluetoothUuid _gattNotifyCharacteristicId(QStringLiteral("49535343-1e4d-4bd9-ba61-23c647249616"));

gpiotreadmill::gpiotreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                                 double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (forceInitSpeed > 0) {
        lastSpeed = forceInitSpeed;
    }

    if (forceInitInclination > 0) {
        lastInclination = forceInitInclination;
    }

    refresh = new QTimer(this);
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &gpiotreadmill::update);
    refresh->start(pollDeviceTime);
}

void gpiotreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                          bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (wait_for_response) {
        connect(this, &gpiotreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        emit debug(QStringLiteral("writeCharacteristic error because the connection is closed"));

        return;
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

void gpiotreadmill::updateDisplay(uint16_t elapsed) {
    uint8_t display[] = {0xf0, 0xcb, 0x03, 0x00, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x05, 0x01, 0x01, 0x00, 0x0c, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00};

    QSettings settings;
    bool distance = settings.value(QStringLiteral("domyos_treadmill_distance_display"), true).toBool();
    bool domyos_treadmill_display_invert = settings.value(QStringLiteral("domyos_treadmill_display_invert"), false).toBool();

    if (elapsed > 5999) // 99:59
    {
        display[3] = ((elapsed / 60) / 60) & 0xFF; // high byte for elapsed time (in seconds)
        display[4] = ((elapsed / 60) % 60) & 0xFF; // low byte for elasped time (in seconds)
    } else {

        display[3] = (elapsed / 60) & 0xFF; // high byte for elapsed time (in seconds)
        display[4] = (elapsed % 60 & 0xFF); // low byte for elasped time (in seconds)
    }

    if (distance) {
        if(!domyos_treadmill_display_invert) {
            if (odometer() < 10.0) {

                display[7] = ((uint8_t)((uint16_t)(odometer() * 100) >> 8)) & 0xFF;
                display[8] = (uint8_t)(odometer() * 100) & 0xFF;
                display[9] = 0x02; // decimal position
            } else if (odometer() < 100.0) {

                display[7] = ((uint8_t)(odometer() * 10) >> 8) & 0xFF;
                display[8] = (uint8_t)(odometer() * 10) & 0xFF;
                display[9] = 0x01; // decimal position
            } else {

                display[7] = ((uint8_t)(odometer()) >> 8) & 0xFF;
                display[8] = (uint8_t)(odometer()) & 0xFF;
                display[9] = 0x00; // decimal position
            }
        } else {
            display[7] = ((uint16_t)(calories().value()) >> 8) & 0xFF;
            display[8] = (uint8_t)(calories().value()) & 0xFF;
            display[9] = 0x00; // decimal position
        }
    } else {

        display[7] = 0x00;
        display[8] = 0x00;
        display[9] = 0x00; // decimal position
    }

    display[12] = (uint8_t)currentHeart().value();

    display[16] = (uint8_t)(currentInclination().value() * 10.0);

    display[20] = (uint8_t)(currentSpeed().value() * 10.0);

    if(!domyos_treadmill_display_invert) {
        display[23] = ((uint8_t)(calories().value()) >> 8) & 0xFF;
        display[24] = (uint8_t)(calories().value()) & 0xFF;
    } else {
        display[23] = ((uint8_t)(odometer() * 10) >> 8) & 0xFF;
        display[24] = (uint8_t)(odometer() * 10) & 0xFF;
    }

    for (uint8_t i = 0; i < sizeof(display) - 1; i++) {

        display[26] += display[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(display, 20, QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, false);
    writeCharacteristic(&display[20], sizeof(display) - 20,
                        QStringLiteral("updateDisplay elapsed=") + QString::number(elapsed), false, true);
}

void gpiotreadmill::forceSpeedOrIncline(double requestSpeed, double requestIncline) {
    uint8_t writeIncline[] = {0xf0, 0xad, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00};

    writeIncline[4] = ((uint16_t)(requestSpeed * 10) >> 8) & 0xFF;
    writeIncline[5] = ((uint16_t)(requestSpeed * 10) & 0xFF);

    writeIncline[13] = ((uint16_t)(requestIncline * 10) >> 8) & 0xFF;
    writeIncline[14] = ((uint16_t)(requestIncline * 10) & 0xFF);

    for (uint8_t i = 0; i < sizeof(writeIncline) - 1; i++) {

        // qDebug() << QString::number(writeIncline[i], 16);
        writeIncline[22] += writeIncline[i]; // the last byte is a sort of a checksum
    }

    // qDebug() << "writeIncline crc" << QString::number(writeIncline[26], 16);

    writeCharacteristic(writeIncline, 20,
                        QStringLiteral("forceSpeedOrIncline speed=") + QString::number(requestSpeed) +
                            " incline=" + QString::number(requestIncline),
                        false, false);
    writeCharacteristic(&writeIncline[20], sizeof(writeIncline) - 20,
                        QStringLiteral("forceSpeedOrIncline speed=") + QString::number(requestSpeed) +
                            QStringLiteral(" incline=") + QString::number(requestIncline),
                        false, true);
}

bool gpiotreadmill::sendChangeFanSpeed(uint8_t speed) {

    uint8_t fanSpeed[] = {0xf0, 0xca, 0x00, 0x00};

    if (speed > 5) {
        return false;
    }

    fanSpeed[2] = speed;

    for (uint8_t i = 0; i < sizeof(fanSpeed) - 1; i++) {

        fanSpeed[3] += fanSpeed[i]; // the last byte is a sort of a checksum
    }

    writeCharacteristic(fanSpeed, 4, QStringLiteral("changeFanSpeed speed=") + QString::number(speed), false, true);

    return true;
}

bool gpiotreadmill::changeFanSpeed(uint8_t speed) {

    requestFanSpeed = speed;

    return true;
}

void gpiotreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void gpiotreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {

        initRequest = false;
        btinit((lastSpeed > 0 ? true : false));
    } else if (/*bluetoothDevice.isValid() &&*/
               m_control->state() == QLowEnergyController::DiscoveredState && gattCommunicationChannelService &&
               gattWriteCharacteristic.isValid() && gattNotifyCharacteristic.isValid() && initDone) {

        QSettings settings;
        // ******************************************* virtual treadmill init *************************************
        if (!firstInit && searchStopped && !virtualTreadMill && !virtualBike) {
            bool virtual_device_enabled = settings.value("virtual_device_enabled", true).toBool();
            bool virtual_device_force_bike = settings.value("virtual_device_force_bike", false).toBool();
            if (virtual_device_enabled) {
                if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    virtualTreadMill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadMill, &virtualtreadmill::debug, this, &gpiotreadmill::debug);
                    connect(virtualTreadMill, &virtualtreadmill::changeInclination, this,
                            &gpiotreadmill::changeInclinationRequested);
                } else {
                    debug("creating virtual bike interface...");
                    virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &gpiotreadmill::changeInclinationRequested);
                }
                firstInit = 1;
            }
        }
        // ********************************************************************************************************

        // debug("Domyos Treadmill RSSI " + QString::number(bluetoothDevice.rssi()));

        double heart = 0;
        QString heartRateBeltName =
            settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    #ifdef Q_OS_ANDROID
        if (settings.value("ant_heart", false).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
    #endif
        {
            if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {

                if (heart == 0) {

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
                } else

                    Heart = heart;
            }
        }

        if (!firstCharacteristicChanged) {
            if (watts(settings.value(QStringLiteral("weight"), 75.0).toFloat()))
                KCal +=
                    ((((0.048 * ((double)watts(settings.value(QStringLiteral("weight"), 75.0).toFloat())) + 1.19) *
                       settings.value(QStringLiteral("weight"), 75.0).toFloat() * 3.5) /
                      200.0) /
                     (60000.0 / ((double)lastTimeCharacteristicChanged.msecsTo(
                                    QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                      // kg * 3.5) / 200 ) / 60
            Distance += ((Speed.value() / (double)3600.0) /
                         ((double)1000.0 / (double)(lastTimeCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
            lastTimeCharacteristicChanged = QDateTime::currentDateTime();
        }

        emit debug(QStringLiteral("Current speed: ") + QString::number(Speed.value()));
        emit debug(QStringLiteral("Current incline: ") + QString::number(Inclination.value()));
        emit debug(QStringLiteral("Current heart: ") + QString::number(Heart.value()));
        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));
        emit debug(QStringLiteral("Current KCal from the machine: ") + QString::number(KCal.value()));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        emit debug(QStringLiteral("Current Distance Calculated: ") + QString::number(Distance.value()));

        if (m_control->error() != QLowEnergyController::NoError) {
            qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
        }

        firstCharacteristicChanged = false;

        update_metrics(true, watts(settings.value(QStringLiteral("weight"), 75.0).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ >= (1000 / refresh->interval())) {
        }

        // byte 3 - 4 = elapsed time
        // byte 17    = inclination
        {
            if (requestSpeed != -1) {
                if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                    emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));

                    double inc = Inclination.value();
                    if (requestInclination != -1) {

                        // only 0.5 steps ara avaiable
                        requestInclination = qRound(requestInclination * 2.0) / 2.0;
                        inc = requestInclination;
                        requestInclination = -1;
                    }
                    forceSpeedOrIncline(requestSpeed, inc);
                }
                requestSpeed = -1;
            }
            if (requestInclination != -1) {
                // only 0.5 steps ara avaiable
                requestInclination = qRound(requestInclination * 2.0) / 2.0;
                if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                    requestInclination <= 15) {
                    emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));

                    double speed = currentSpeed().value();
                    if (requestSpeed != -1) {

                        speed = requestSpeed;
                        requestSpeed = -1;
                    }
                    forceSpeedOrIncline(speed, requestInclination);
                }
                requestInclination = -1;
            }
            if (requestStart != -1) {
                emit debug(QStringLiteral("starting..."));
                if (lastSpeed == 0.0) {

                    lastSpeed = 0.5;
                }
                requestStart = -1;
                emit tapeStarted();
            }
            if (requestStop != -1) {
                emit debug(QStringLiteral("stopping..."));
                writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), QStringLiteral("stop tape"), false,
                                    true);
                requestStop = -1;
            }
            if (requestFanSpeed != -1) {
                emit debug(QStringLiteral("changing fan speed..."));

                sendChangeFanSpeed(requestFanSpeed);
                requestFanSpeed = -1;
            }
            if (requestIncreaseFan != -1) {
                emit debug(QStringLiteral("increasing fan speed..."));

                sendChangeFanSpeed(FanSpeed + 1);
                requestIncreaseFan = -1;
            } else if (requestDecreaseFan != -1) {
                emit debug(QStringLiteral("decreasing fan speed..."));

                sendChangeFanSpeed(FanSpeed - 1);
                requestDecreaseFan = -1;
            }
        }
    }
}

bool gpiotreadmill::connected() {
    return true;
}

void *gpiotreadmill::VirtualTreadMill() { return virtualTreadMill; }

void *gpiotreadmill::VirtualDevice() { return VirtualTreadMill(); }

void gpiotreadmill::searchingStop() { searchStopped = true; }
