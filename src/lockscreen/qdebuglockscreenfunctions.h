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

    void setVirtualBike(bool zwiftMode) override;

    void setVirtualTreadmill(bool zwiftMode) override;

    void setVirtualRower(bool isVirtualDevice) override;

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

