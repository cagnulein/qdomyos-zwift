#pragma once


#include "qzlockscreenfunctions.h"

class qdebuglockscreenfunctions : public QZLockscreenFunctions
{
private:
    enum configurationType {
        NONE, BIKE, TREADMILL, ROWER
    };

    configurationType configuredFor = configurationType::NONE;
    void checkConfigured();
public:
    qdebuglockscreenfunctions(bool pelotonWorkaroundActive);

    void setVirtualBikePelotonWorkaround(bool zwiftMode) override;

    void setVirtualTreadmillPelotonWorkaround(bool zwiftMode) override;

    void setVirtualRowerPelotonWorkaround(bool isVirtualDevice) override;

    QZLockscreen * getLockscreen() const override;

    bool updateEnergyDistanceHeartRate(metric kcal, metric distance, metric &heart, const int defaultHeartRate=0) override;

    bool updateHeartRate(metric &heart) override;

    void updateStepCadence(metric& cadence) override;

    bool isPelotonWorkaroundActive() const override;

    void pelotonBikeUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) override;

    void pelotonTreadmillUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) override;

    void pelotonRowerUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) override;
private:
    bool pelotonWorkaroundActive = false;
    void pelotonUpdateCHR(const QString &deviceType, const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate);
};

