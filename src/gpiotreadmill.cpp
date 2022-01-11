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
#include <wiringPi.h>

using namespace std::chrono_literals;

gpiotreadmill::gpiotreadmill(uint32_t pollDeviceTime, bool noConsole, bool noHeartService, double forceInitSpeed,
                             double forceInitInclination) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    this->noConsole = noConsole;
    this->noHeartService = noHeartService;

    if (wiringPiSetup() == -1) {
        qDebug() << QStringLiteral("wiringPiSetup ERROR!");
        exit(1);
    }
    pinMode(OUTPUT_INCLINE_DOWN, OUTPUT);
    pinMode(OUTPUT_INCLINE_UP, OUTPUT);
    pinMode(OUTPUT_SPEED_DOWN, OUTPUT);
    pinMode(OUTPUT_SPEED_UP, OUTPUT);
    pinMode(OUTPUT_START, OUTPUT);
    pinMode(OUTPUT_STOP, OUTPUT);
    digitalWrite(OUTPUT_INCLINE_DOWN, 0);
    digitalWrite(OUTPUT_INCLINE_UP, 0);
    digitalWrite(OUTPUT_SPEED_DOWN, 0);
    digitalWrite(OUTPUT_SPEED_UP, 0);
    digitalWrite(OUTPUT_START, 0);
    digitalWrite(OUTPUT_STOP, 0);

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

void gpiotreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void gpiotreadmill::forceSpeed(double requestSpeed) {
    if (requestSpeed > Speed.value()) {
        digitalWrite(OUTPUT_SPEED_UP, 1);
        QThread::sleep(GPIO_KEEP_MS);
        digitalWrite(OUTPUT_SPEED_UP, 0);
    } else {
        digitalWrite(OUTPUT_SPEED_DOWN, 1);
        QThread::sleep(GPIO_KEEP_MS);
        digitalWrite(OUTPUT_SPEED_DOWN, 0);
    }
    Speed = requestSpeed;
}

void gpiotreadmill::forceIncline(double requestIncline) {
    if (requestIncline > Inclination.value()) {
        digitalWrite(OUTPUT_INCLINE_UP, 1);
        QThread::sleep(GPIO_KEEP_MS);
        digitalWrite(OUTPUT_INCLINE_UP, 0);
    } else {
        digitalWrite(OUTPUT_INCLINE_DOWN, 1);
        QThread::sleep(GPIO_KEEP_MS);
        digitalWrite(OUTPUT_INCLINE_DOWN, 0);
    }
    Inclination = requestIncline;
}

void gpiotreadmill::update() {

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
                connect(virtualBike, &virtualbike::changeInclination, this, &gpiotreadmill::changeInclinationRequested);
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

                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -1) {
            // only 0.5 steps ara avaiable
            requestInclination = qRound(requestInclination * 2.0) / 2.0;
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));

                forceIncline(requestInclination);
            }
            requestInclination = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            digitalWrite(OUTPUT_START, 1);
            QThread::sleep(GPIO_KEEP_MS);
            digitalWrite(OUTPUT_START, 0);
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            digitalWrite(OUTPUT_STOP, 1);
            QThread::sleep(GPIO_KEEP_MS);
            digitalWrite(OUTPUT_STOP, 0);
            requestStop = -1;
        }
        if (requestFanSpeed != -1) {
            emit debug(QStringLiteral("changing fan speed..."));

            requestFanSpeed = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));

            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));

            requestDecreaseFan = -1;
        }
    }
}

bool gpiotreadmill::connected() { return true; }

void *gpiotreadmill::VirtualTreadMill() { return virtualTreadMill; }

void *gpiotreadmill::VirtualDevice() { return VirtualTreadMill(); }

void gpiotreadmill::searchingStop() { searchStopped = true; }
