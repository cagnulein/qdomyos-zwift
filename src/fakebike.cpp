#include "fakebike.h"
#include "ios/lockscreen.h"
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
#include "keepawakehelper.h"
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


    // ******************************************* virtual bike init *************************************
    if (!firstStateChanged && !virtualBike && !noVirtualDevice
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
            virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
            connect(virtualBike, &virtualbike::changeInclination, this,
                    &fakebike::changeInclinationRequested);
            connect(virtualBike, &virtualbike::ftmsCharacteristicChanged, this, &fakebike::ftmsCharacteristicChanged);
        }
    }
    if(!firstStateChanged) emit connectedAndDiscovered();
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

bool fakebike::connected() {
    return true;
}

void *fakebike::VirtualBike() { return virtualBike; }

void *fakebike::VirtualDevice() { return VirtualBike(); }
