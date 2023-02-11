#ifndef QZLOCKSCREENFUNCTIONS_H
#define QZLOCKSCREENFUNCTIONS_H

#include "metric.h"
#include "qzlockscreen.h"

/**
 * @brief The QZLockscreenFunctions class provides an interface for some common functions for updating an object
 * representing a IOS device with a lockscreen.
 */
class QZLockscreenFunctions {
protected:
    QZLockscreenFunctions() {}
public:
    virtual ~QZLockscreenFunctions() {}

    /**
     * @brief Perform configuration for a bike.
     * @param zwiftMode Indicates if the Zwift interface should be used.
     */
    virtual void setVirtualBike(bool zwiftMode)=0;

    /**
     * @brief Perform configuration for a treadmill.
     * @param zwiftMode Indicates if the Zwift interface should be used.
     */
    virtual void setVirtualTreadmill(bool zwiftMode)=0;

    /**
     * @brief Perform configuration for a rower.
     * @param isVirtualDevice Indicates if this is for a virtual device.
     */
    virtual void setVirtualRower(bool isVirtualDevice)=0;

    /**
     * @brief Gets the implementation of QZLockscreen for accessing lockscreen functions directly.
     */
    virtual QZLockscreen * getLockscreen() const =0;

    /**
     * @brief Updates the kcal and distance on the lockscreen, and gets the heart rate from it.
     * @param kcal
     * @param distance
     * @param heart
     * @param defaultHeartRate
     * @return
     */
    virtual bool updateEnergyDistanceHeartRate(metric kcal, metric distance, metric &heart, const int defaultHeartRate=0)=0;

    /**
     * @brief Get the heart rate from the lockscreen.
     * @param heart
     * @return True if the update occurred, false if not.
     */
    virtual bool updateHeartRate(metric &heart)=0;

    /**
     * @brief Gets the cadence from the lockscreen.
     * @param cadence The metric object to update.
     */
    virtual void updateStepCadence(metric& cadence) = 0;

    /**
     * @brief Indicates if the Peloton workaround is active.
     * @return
     */
    virtual bool isPelotonWorkaroundActive() const;

    /**
     * @brief Update the crank and heart rate for a bike on the lockscreen if the Peloton workaround is active.
     * @param crankRevolutions
     * @param lastCrankEventTime
     * @param heartRate
     */
    virtual void pelotonBikeUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate)=0;

    /**
     * @brief Update the crank and heart rate for a treadmill on the lockscreen if the Peloton workaround is active.
     * @param crankRevolutions
     * @param lastCrankEventTime
     * @param heartRate
     */
    virtual void pelotonTreadmillUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate)=0;

    /**
     * @brief Update the crank and heart rate for a rower on the lockscreen if the Peloton workaround is active.
     * @param crankRevolutions
     * @param lastCrankEventTime
     * @param heartRate
     */
    virtual void pelotonRowerUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate)=0;

    /**
     * @brief Tries to create a QZLockscreen object.
     * @return nullptr if there is no lockscreen to update, in implementation of QZLockscreenFunctions otherwise.
     */
    static QZLockscreenFunctions * create();
};

#endif // QZLOCKSCREENFUNCTIONS_H
