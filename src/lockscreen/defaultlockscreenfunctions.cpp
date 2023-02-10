#include "lockscreen/defaultlockscreenfunctions.h"



DefaultLockscreenFunctions::DefaultLockscreenFunctions() {}

QZLockscreen *DefaultLockscreenFunctions::getLockscreen() const {
    throw "Not supported on this platform.";
}

void DefaultLockscreenFunctions::setVirtualBike(bool zwiftMode) {}

void DefaultLockscreenFunctions::setVirtualTreadmill(bool zwiftMode) {}

void DefaultLockscreenFunctions::setVirtualRower(bool isVirtualDevice) {}

bool DefaultLockscreenFunctions::isPelotonWorkaroundActive() const { return false; }

bool DefaultLockscreenFunctions::updateEnergyDistanceHeartRate(metric kcal, metric distance, metric &heart, const int defaultHeartRate) { return false;}

bool DefaultLockscreenFunctions::updateHeartRate(metric &heart) {return false;}

void DefaultLockscreenFunctions::updateStepCadence(metric &cadence) {}

void DefaultLockscreenFunctions::pelotonBikeUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {}

void DefaultLockscreenFunctions::pelotonTreadmillUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {}

void DefaultLockscreenFunctions::pelotonRowerUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate) {}
