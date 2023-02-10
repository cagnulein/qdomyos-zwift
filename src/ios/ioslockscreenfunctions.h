#ifndef IOSLOCKSCREENFUNCTIONS_H
#define IOSLOCKSCREENFUNCTIONS_H

#include <qglobal.h>

#include "lockscreen/qzlockscreenfunctions.h"

class IOSLockscreenFunctions : public QZLockscreenFunctions {
private:
    bool pelotonWorkaroundActive = false;
    QZLockscreen * lockscreen = nullptr;
public:
    explicit IOSLockscreenFunctions();

    QZLockscreen * getLockscreen() const override;

    void setVirtualBike(bool zwiftMode) override;
    void setVirtualTreadmill(bool zwiftMode) override;
    void setVirtualRower(bool isVirtualDevice) override;

    bool isPelotonWorkaroundActive() const override;
    bool updateEnergyDistanceHeartRate(metric kcal, metric distance, metric &heart, const int defaultHeartRate) override;
    bool updateHeartRate(metric &heart) override;
    void updateStepCadence(metric& cadence) override;
    void pelotonBikeUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) override;
    void pelotonTreadmillUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) override;
    void pelotonRowerUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) override;
};


#endif // IOSLOCKSCREENFUNCTIONS_H
