#include <QDebug>
#include "qdebuglockscreenfunctions.h"

qdebuglockscreenfunctions::qdebuglockscreenfunctions(bool pelotonWorkaroundActive)
{
    this->pelotonWorkaroundActive = pelotonWorkaroundActive;
}

void qdebuglockscreenfunctions::checkConfigured() {
    if(this->configuredFor==configurationType::NONE) {
        qDebug() << "Called a lockscreen function on an unconfigured lockscreen functions object.";
    }
}

void qdebuglockscreenfunctions::setVirtualBikePelotonWorkaround(bool zwiftMode) {
    qDebug() << "Configuring qdebug lockscreen for virtual bike. Zwift mode: " << zwiftMode;
    this->configuredFor = configurationType::BIKE;
}

void qdebuglockscreenfunctions::setVirtualTreadmillPelotonWorkaround(bool zwiftMode) {
    qDebug() << "Configuring qdebug lockscreen for virtual treadmill. Zwift mode: " << zwiftMode;
    this->configuredFor = configurationType::TREADMILL;
}

void qdebuglockscreenfunctions::setVirtualRowerPelotonWorkaround(bool isVirtualDevice) {
    qDebug() << "Configuring qdebug lockscreen for virtual rower. isVirtualDevice: " << isVirtualDevice;
    this->configuredFor = configurationType::ROWER;
}

QZLockscreen *qdebuglockscreenfunctions::getLockscreen() const {
    return nullptr;
}

bool qdebuglockscreenfunctions::updateEnergyDistanceHeartRate(metric kcal, metric distance, metric &heart, const int defaultHeartRate) {
    this->checkConfigured();
    qDebug() << "updating qdebug lockscreen:  "
             << kcal.value() << "kcal, "
             <<  distance.value() << "km, "
              << heart.value() << "BPM, "
              << "default HR=" << defaultHeartRate << "BPM";
}

bool qdebuglockscreenfunctions::updateHeartRate(metric &heart) {
    this->checkConfigured();
    qDebug() << "updating qdebug lockscreen:  "
             << heart.value() << "BPM, ";
}

void qdebuglockscreenfunctions::updateStepCadence(metric &cadence) {
    this->checkConfigured();
    qDebug() << "updating qdebug lockscreen step cadence:  "
             << cadence.value() ;
    if(this->configuredFor!=configurationType::TREADMILL) {
        qDebug() << "WARNING: lockscreen step cadence update called when not configured as a virtual treadmill.";
    }
}

bool qdebuglockscreenfunctions::isPelotonWorkaroundActive() const { return this->pelotonWorkaroundActive; }

void qdebuglockscreenfunctions::pelotonBikeUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {
    this->checkConfigured();
    if(this->configuredFor!=configurationType::BIKE) {
        qDebug() << "WARNING: lockscreen peloton workaround update for bike called when not configured as a virtual bike.";
    }
    this->pelotonUpdateCHR(QStringLiteral("bike"), crankRevolutions, lastCrankEventTime, heartRate);
}

void qdebuglockscreenfunctions::pelotonTreadmillUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {
    this->checkConfigured();
    if(this->configuredFor!=configurationType::TREADMILL) {
        qDebug() << "WARNING: lockscreen peloton workaround update for treadmill called when not configured as a virtual treadmill.";
    }
    this->pelotonUpdateCHR(QStringLiteral("treadmill"), crankRevolutions, lastCrankEventTime, heartRate);
}

void qdebuglockscreenfunctions::pelotonRowerUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {
    this->checkConfigured();
    if(this->configuredFor!=configurationType::ROWER) {
        qDebug() << "WARNING: lockscreen peloton workaround update for a rower called when not configured as a virtual rower.";
    }

    this->pelotonUpdateCHR(QStringLiteral("rower"), crankRevolutions, lastCrankEventTime, heartRate);
}

void qdebuglockscreenfunctions::pelotonUpdateCHR(const QString& deviceType, const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {
    qDebug() << "Peloton workaround for " << deviceType << ",  updating crank and heart rate: "
             << crankRevolutions << "RPM, "
             << "last crank event time " << lastCrankEventTime << " "
             << "hear rate " << heartRate << "BPM"   ;
}
