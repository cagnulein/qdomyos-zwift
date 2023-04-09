#include "lockscreenfunctions.h"
#include "qzsettings.h"
#include <QSettings>
#include "objectfactory.h"


LockscreenFunctions::LockscreenFunctions() : QZLockscreenFunctions() {
    this->lockscreen = ObjectFactory::createLockscreen();
    this->pelotonWorkaroundActive = false;
}

QZLockscreen *LockscreenFunctions::getLockscreen() const { return this->lockscreen; }

bool LockscreenFunctions::tryConfigurePelotonWorkaround(configurationType configType, bool zwiftMode) {
    this->pelotonWorkaroundActive = false;

    if(configType==configurationType::BIKE)
        this->setVirtualBikePelotonWorkaround(zwiftMode);
    else if (configType==configurationType::TREADMILL)
        this->setVirtualTreadmillPelotonWorkaround(zwiftMode);
    else if (configType==configurationType::ROWER)
        this->setVirtualRowerPelotonWorkaround(zwiftMode);
    else if (configType!=configurationType::NONE)
        throw "Unrecognised configuration type.";

    this->configType = configType;

    return this->pelotonWorkaroundActive;
}

QZLockscreenFunctions::configurationType LockscreenFunctions::getConfigurationType() const {
    return this->configType;
}


void LockscreenFunctions::setVirtualBikePelotonWorkaround(bool zwiftMode) {

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

            bool useBikeHeartRateService = settings.value(QZSettings::bike_heartrate_service, QZSettings::default_bike_heartrate_service).toBool();
            this->getLockscreen()->virtualbike_zwift_ios(useBikeHeartRateService);
            this->pelotonWorkaroundActive = true;
        }
    }
}

void LockscreenFunctions::setVirtualTreadmillPelotonWorkaround(bool zwiftMode) {

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

void LockscreenFunctions::setVirtualRowerPelotonWorkaround(bool zwiftMode) {

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
            this->pelotonWorkaroundActive = true;
        }
    } else {

        QSettings settings;

        bool heart_only =
            settings.value(QZSettings::virtual_device_onlyheart, QZSettings::default_virtual_device_onlyheart).toBool();
        if (ios_peloton_workaround && !heart_only) {

            qDebug() << "ios_zwift_workaround activated!";

            this->getLockscreen()->virtualrower_ios();
            this->pelotonWorkaroundActive = true;
        }
    }

}


bool LockscreenFunctions::isPelotonWorkaroundActive() const {
    return this->pelotonWorkaroundActive;
}

bool LockscreenFunctions::updateEnergyDistance(metric kcal, metric distance) {
    QZLockscreen * ls = this->getLockscreen();
    if(!ls)
        return false;

    ls->setKcal(kcal.value());
    ls->setDistance(distance.value());

    return true;
}

bool LockscreenFunctions::updateHeartRate(metric &heart, int defaultHeartRate) {
    long appleWatchHeartRate = 0;

    QZLockscreen * ls = this->getLockscreen();
    if(!ls)
        return false;

    appleWatchHeartRate = ls->heartRate();

    if (appleWatchHeartRate == 0)
        heart = defaultHeartRate;
    else
        heart = appleWatchHeartRate;

    qDebug() << "Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate);

    return true;
}

void LockscreenFunctions::updateStepCadence(metric &cadence) {

    QSettings settings;
    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
            .toString()
            .startsWith(QStringLiteral("Disabled")))
    {
        QZLockscreen * ls = this->getLockscreen();
        if(!ls) return;
        long appleWatchCadence = ls->stepCadence();
        cadence = appleWatchCadence;
    }

}


void LockscreenFunctions::pelotonUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {

    if(!this->isPelotonWorkaroundActive())
        return;

    // assuming that because the Peloton workaround is active, there is a lockscreen object.

    QZLockscreen * lockscreen = this->getLockscreen();
    auto configType = this->getConfigurationType();

    if(configType==QZLockscreenFunctions::configurationType::BIKE) {
        lockscreen->virtualbike_setCadence(crankRevolutions, lastCrankEventTime);
        lockscreen->virtualbike_setHeartRate(heartRate);
    } else if (configType==QZLockscreenFunctions::configurationType::TREADMILL) {
        lockscreen->virtualbike_setCadence(crankRevolutions, lastCrankEventTime);
        lockscreen->virtualbike_setHeartRate(heartRate);

        // lockscreen->virtualTreadmill_setCadence(currentCrankRevolutions, lastCrankEventTime);
        // lockscreen->virtualTreadmill_setHeartRate(heartRate);

    } else if (configType==QZLockscreenFunctions::configurationType::ROWER) {
        QSettings settings;
        bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
        bool virtual_device_rower = settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
        if (cadence && !virtual_device_rower) {
            lockscreen->virtualbike_setCadence(crankRevolutions, lastCrankEventTime);
            lockscreen->virtualbike_setHeartRate(heartRate);
        }
    } else {
        qDebug() << "pelotonUpdateCHR called on unconfigured lockscreen functions object";
    }

}

