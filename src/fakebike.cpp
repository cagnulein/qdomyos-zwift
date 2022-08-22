#include "fakebike.h"
#include "virtualbike.h"
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
#include <chrono>

using namespace std::chrono_literals;

fakebike::fakebike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &fakebike::update);
    refresh->start(200ms);
}

void fakebike::update() {
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    update_metrics(true, watts());

    Distance += ((Speed.value() / (double)3600.0) /
                 ((double)1000.0 / (double)(lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime()))));
    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice() && !noVirtualDevice
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {
        bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence = settings.value("bike_cadence_sensor", false).toBool();
        bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
        if (ios_peloton_workaround && cadence) {
            qDebug() << "ios_peloton_workaround activated!";
            h = new lockscreen();
            h->virtualbike_ios();
        } else
#endif
#endif
        if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual bike interface..."));
            auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
            connect(virtualBike, &virtualbike::changeInclination, this, &fakebike::changeInclinationRequested);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &fakebike::ftmsCharacteristicChanged);
            this->setVirtualDevice(virtualBike, false);
        }
    }
    if (!firstStateChanged)
        emit connectedAndDiscovered();
    firstStateChanged = 1;
    // ********************************************************************************************************

    if (!noVirtualDevice) {
#ifdef Q_OS_ANDROID
        if (settings.value("ant_heart", false).toBool()) {
            Heart = (uint8_t)KeepAwakeHelper::heart();
            debug("Current Heart: " + QString::number(Heart.value()));
        }
#endif
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

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        bool cadence = settings.value("bike_cadence_sensor", false).toBool();
        bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
        if (ios_peloton_workaround && cadence && h && firstStateChanged) {
            h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
            h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
        }
#endif
#endif
    }

    if (Heart.value()) {

        /*
         * If VO2 max is unknown, the following formulas would apply:

         Women:

            CB = T * (0.4472*H - 0.1263*W + 0.074*A - 20.4022) / 4.184

         Men:

            CB = T * (0.6309*H + 0.1988*W + 0.2017*A - 55.0969) / 4.184

        Where:

        CB is the number of calories burned;
        T is the duration of exercise in minutes;
        H is your average heart rate in beats per minute;
        W is your weight in kilograms; and
            A is your age in years.
        */

        QString sex = settings.value(QStringLiteral("sex"), "Male").toString();
        double weight = settings.value(QStringLiteral("weight"), 75.0).toFloat();
        double age = settings.value(QStringLiteral("age"), 35).toDouble();
        double T = elapsed.value() / 60;
        double H = Heart.average();
        double W = weight;
        double A = age;
        if (sex.toLower().contains("female")) {
            KCal = T * ((0.4472 * H) - (0.1263 * W) + (0.074 * A) - 20.4022) / 4.184;
        } else {
            KCal = T * ((0.6309 * H) + (0.1988 * W) + (0.2017 * A) - 55.0969) / 4.184;
        }
    }

    if (requestResistance != -1 && requestResistance != currentResistance().value()) {
        Resistance = requestResistance;
        m_pelotonResistance = requestResistance;
    }
}

void fakebike::ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QByteArray b = newValue;
    qDebug() << "routing FTMS packet to the bike from virtualbike" << characteristic.uuid() << newValue.toHex(' ');
}

void fakebike::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

bool fakebike::connected() { return true; }

