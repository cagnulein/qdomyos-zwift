#include "ioslockscreenfunctions.h"
#include "qzsettings.h"
#include <QSettings>

#undef IOS_ENABLED
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
#include "ios/lockscreen.h"
#define IOS_ENABLED 1
#endif
#endif

IOSLockscreenFunctions::IOSLockscreenFunctions() : QZLockscreenFunctions() {

    // prevent instantiation on the wrong platform
#ifndef IOS_ENABLED
    throw "Not supported on this platform";
#else
    this->lockscreen = new lockscreen();
#endif
}

QZLockscreen *IOSLockscreenFunctions::getLockscreen() const { return this->lockscreen; }


void IOSLockscreenFunctions::setVirtualBike(bool zwiftMode) {

    this->pelotonWorkaroundActive = false;

    QSettings settings;
    bool ios_peloton_workaround =
            settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();

    if(!ios_peloton_workaround) return;

    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();

    if(!zwiftMode) {
        if (cadence) {
            qDebug() << "ios_peloton_workaround activated!";

            this->getLockscreen()->virtualbike_ios();

            this->pelotonWorkaroundActive = true;
        }
    } else {
        bool power = settings.value(QZSettings::bike_power_sensor, QZSettings::default_bike_power_sensor).toBool();
        bool heart_only = settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();
        bool echelon = settings.value(QZSettings::virtual_device_echelon, QZSettings::default_virtual_device_echelon).toBool();
        bool ifit = settings.value(QZSettings::virtual_device_ifit, QZSettings::default_virtual_device_ifit).toBool();

        if (!cadence && !echelon && !ifit && !heart_only && !power) {

            qDebug() << "ios_zwift_workaround activated!";

            this->getLockscreen()->virtualbike_zwift_ios();
        }
    }
}

void IOSLockscreenFunctions::setVirtualTreadmill(bool zwiftMode) {

    this->pelotonWorkaroundActive = false;

    QSettings settings;
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround) {
        qDebug() << "ios_zwift_workaround activated!";

        if(zwiftMode)
            // orignally from virtualtreadmill
            this->getLockscreen()->virtualtreadmill_zwift_ios();
        else
            this->getLockscreen()->virtualbike_ios();

        this->pelotonWorkaroundActive = true;
    }
}

void IOSLockscreenFunctions::setVirtualRower(bool zwiftMode) {

    this->pelotonWorkaroundActive = false;

    QSettings settings;

    bool ios_peloton_workaround = settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();

    if(!ios_peloton_workaround)
        return;



    if(!zwiftMode) {

        bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();

        bool virtual_device_rower = settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
        if (ios_peloton_workaround && cadence && !virtual_device_rower) {
            qDebug() << "ios_peloton_workaround activated!";
            this->getLockscreen()->virtualbike_ios();
        }
    } else {

        QSettings settings;

        bool heart_only =
            settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();
        if (ios_peloton_workaround && !heart_only) {

            qDebug() << "ios_zwift_workaround activated!";

            this->getLockscreen()->virtualrower_ios();

        }
    }

}


bool IOSLockscreenFunctions::isPelotonWorkaroundActive() const {
    return this->pelotonWorkaroundActive;
}

bool IOSLockscreenFunctions::updateEnergyDistanceHeartRate(metric kcal, metric distance, metric &heart, int defaultHeartRate) {

    long appleWatchHeartRate = 0;
    QZLockscreen * ls = this->getLockscreen();

    appleWatchHeartRate = ls->heartRate();
    ls->setKcal(kcal.value());
    ls->setDistance(distance.value());

    if (appleWatchHeartRate == 0)
        heart = defaultHeartRate;
    else
        heart = appleWatchHeartRate;

    qDebug() << "Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate);

    //debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
    return true;
}

bool IOSLockscreenFunctions::updateHeartRate(metric &heart) {
    long appleWatchHeartRate = 0;

    appleWatchHeartRate = this->getLockscreen()->heartRate();

    heart = appleWatchHeartRate;

    qDebug() << "Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate);

    //debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
    return true;
}

void IOSLockscreenFunctions::updateStepCadence(metric &cadence) {

    QSettings settings;
    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled")))
    {
        long appleWatchCadence = this->getLockscreen()->stepCadence();
        cadence = appleWatchCadence;
    }

}

void IOSLockscreenFunctions::pelotonBikeUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {

    if(!this->isPelotonWorkaroundActive())
        return;

    this->getLockscreen()->virtualbike_setCadence(crankRevolutions, lastCrankEventTime);
    this->getLockscreen()->virtualbike_setHeartRate(heartRate);

}

void IOSLockscreenFunctions::pelotonTreadmillUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {

    return pelotonBikeUpdateCHR(crankRevolutions, lastCrankEventTime, heartRate);

/*
    if(!this->isPelotonWorkaroundActive())
        return;

    this->getLockscreen()->virtualTreadmill_setCadence(currentCrankRevolutions, lastCrankEventTime);
    this->getLockscreen()->virtualTreadmill_setHeartRate(heartRate);
    */

}

void IOSLockscreenFunctions::pelotonRowerUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {

    if(!this->isPelotonWorkaroundActive())
        return;

    QSettings settings;
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool virtual_device_rower = settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
    if (cadence && !virtual_device_rower) {
        this->getLockscreen()->virtualbike_setCadence(crankRevolutions, lastCrankEventTime);
        this->getLockscreen()->virtualbike_setHeartRate(heartRate);
    }

}
