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

    enum configurationType {
        NONE, BIKE, TREADMILL, ROWER
    };

    virtual ~QZLockscreenFunctions() {}

    /**
     * @brief Try to configure the Peloton workaround using the conditions for the specified configuratipn type.
     * Determines if the Peloton workaround should be active.
     * @param configType The type of device to configure for.
     * @param zwiftMode Indicates if the Zwift interface should be used.
     */
    virtual bool tryConfigurePelotonWorkaround(configurationType configType, bool zwiftMode) = 0;

    /**
     * @brief Gets the implementation of QZLockscreen for accessing lockscreen functions directly.
     */
    virtual QZLockscreen * getLockscreen() const =0;

    /**
     * @brief Gets the configuration type for the Peloton workaround.
     * @return
     */
    virtual configurationType getConfigurationType() const =0;

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
     * @brief Update the crank and heart rate on the lockscreen if the Peloton workaround is active.
     * @param crankRevolutions
     * @param lastCrankEventTime
     * @param heartRate
     */
    virtual void pelotonUpdateCHR(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate)=0;

};

#endif // QZLOCKSCREENFUNCTIONS_H
