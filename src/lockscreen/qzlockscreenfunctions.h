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
     * @brief Updates the kcal and distance on the lockscreen.
     * @param kcal
     * @param distance
          * @return True if the update occurred, false if not.
     */
    virtual bool setEnergyDistance(metric kcal, metric distance)=0;

    /**
     * @brief Get the heart rate from the lockscreen.
     * @param heart
     * @param defaultHeartRate The defult heart rate to set if none is provided by the external device.
     * @return True if the update occurred, false if not.
     */
    virtual bool getHeartRate(metric &heart, const int defaultHeartRate=0)=0;

    /**
     * @brief Gets the cadence from the lockscreen.
     * @param cadence The metric object to update.
     */
    virtual void getStepCadence(metric& cadence) = 0;

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
    virtual void pelotonSetCrankHeartRate(const double crankRevolutions, const uint16_t lastCrankEventTime, const uint8_t heartRate)=0;

};

#endif // QZLOCKSCREENFUNCTIONS_H
