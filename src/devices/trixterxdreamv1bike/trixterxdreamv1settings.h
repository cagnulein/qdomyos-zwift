#ifndef TRIXTERXDREAMV1SETTINGS_H
#define TRIXTERXDREAMV1SETTINGS_H


#include <cstdint>
#include "qmutex.h"
#include "qsettings.h"
#include "qzsettings.h"


/**
 * @brief The trixterxdreamv1bikesettings class encapsulates the application settings for the Trixter X-Dream V1 Bike.
 * Field accessors restrict values to defined limits.
 */
class trixterxdreamv1settings {
public:
    // these should match the corresponding values in settings.qml
    // - the default values where the properties are defined
    constexpr static int8_t MaxSteeringAngle = 45;
    constexpr static uint16_t MinConnectionTimeoutMilliseconds = 20;
    constexpr static uint16_t MaxConnectionTimeoutMilliseconds = 10000;
    constexpr static bool DefaultEnabled =true;
    constexpr static bool DefaultSteeringEnabled =true;
    constexpr static bool DefaultHeartRateEnabled =true;
    constexpr static int8_t DefaultSteeringCalibrationL = -MaxSteeringAngle;
    constexpr static int8_t DefaultSteeringCalibrationR = MaxSteeringAngle;
    constexpr static int8_t DefaultSteeringCalibrationCL = -2;
    constexpr static int8_t DefaultSteeringCalibrationCR = 2;
    constexpr static uint16_t DefaultConnectionTimeoutMilliseconds = 500;


    struct steeringCalibrationInfo {

    public:

        /**
         * @brief left The uncalibrated left turning angle that will be mapped to hard left.
         */
        int8_t left;

        /**
         * @brief centerLeft The lower uncalibrated left turning angle that will be mapped to 0.
         */
        int8_t centerLeft;

        /**
         * @brief centerRight The higest uncalibrated right turning angle that will be mapped to 0.
         */
        int8_t centerRight;

        /**
         * @brief right The uncalibrated right turning angle that will be mapped to hard right.
         */
        int8_t right;

        /**
         * @brief max The maximum turning angle.
         */
        static const int8_t max = MaxSteeringAngle;

        /**
         * @brief isValid Validates the record.
         * @return True if the values are valid, false otherwise.
         */
        bool isValid() const {
            return left>=-max && left<centerLeft && centerLeft<centerRight && centerRight<right && right<=max;
        }

        steeringCalibrationInfo()
        :left(DefaultSteeringCalibrationL), centerLeft(DefaultSteeringCalibrationCL),
          centerRight(DefaultSteeringCalibrationCR), right(DefaultSteeringCalibrationR)
        {

        }

        steeringCalibrationInfo(const int8_t left, const int8_t centerLeft, const int8_t centerRight, const int8_t right)
            :left(left), centerLeft(centerLeft), centerRight(centerRight), right(right)
        {
            if(!this->isValid())
                throw "Arguments are out of range or out of order.";
        }

        friend bool operator==(const steeringCalibrationInfo& lhs, const steeringCalibrationInfo& rhs)
        {
            return rhs.left==lhs.left &&
                   rhs.right==lhs.right &&
                   rhs.centerLeft==lhs.centerLeft &&
                   rhs.centerRight==lhs.centerRight;

        }
    };

private:
    // mutex for thread syncing, may attempt double lock when loading from QSettings, so using recursive mutex
    QRecursiveMutex mutex;
    bool enabled=DefaultEnabled;
    bool steeringEnabled = DefaultSteeringEnabled;
    bool heartRateEnabled = DefaultHeartRateEnabled;
    uint16_t connectionTimeoutMilliseconds = DefaultConnectionTimeoutMilliseconds;

    steeringCalibrationInfo steeringCalibration;

    uint32_t version=0;

    /**
     * @brief clip Clips the value to be within the specified minimum and maximum.
     * @param minimum The minimum value.
     * @param maximum The maximum value.
     * @param value The value to clip.
     */
    template <typename T>
    static T clip(const T minimum, const T maximum, const T value) { return std::max(minimum, std::min(maximum, value)); }

    /**
     * @brief updateField Updates a field and increments the version if the value has changed.
     * @param member The member to update.
     * @param newValue The new value.
     * @return The value set.
     */
    template <typename T>
    T updateField(T& member, const T newValue);

public:

    /**
     * @brief get_version Incremented if the values are modified.
     */
    uint32_t get_version();

    /**
     * @brief get_enabled Indicates if the device is enabled, i.e. should be searched for.
     */
    bool get_enabled();

    /**
     * @brief set_enabled Sets whether or not the type of device is enabled in the application.
     * @param value The value to set.
     * @return The actual value set.
     */
    bool set_enabled(bool value);

    /**
     * @brief get_heartRateEnabled Indicates if the the heart rate signal is enabled.
     */
    bool get_heartRateEnabled();

    /**
     * @brief set_heartRateEnabled Enables/disables steering.
     * @param value True to use heart rate data, false to ignore it.
     * @return The actual value set;
     */
    bool set_heartRateEnabled(bool value);

    /**
     * @brief get_steeringEnabled Indicates if the steering is enabled.
     */
    bool get_steeringEnabled();

    /**
     * @brief set_steeringEnabled Enables/disables steering.
     * @param value True to use steering data, false to ignore it.
     * @return The actual value set;
     */
    bool set_steeringEnabled(bool value);

    /**
     * @brief get_steeringCalibration Gets the values for steering calibration.
     */
    steeringCalibrationInfo get_steeringCalibration();

    /**
     * @brief set_steeringCalibration sets the values for steering calibration.
     * @param value The calibraion values.
     */
    void set_steeringCalibration(const steeringCalibrationInfo value);

    /**
     * @brief get_ConnectionTimeoutMilliseconds Gets the number of milliseconds the
     * detector will wait for valid data from the serial port.
     * @return
     */
    uint16_t get_connectionTimeoutMilliseconds();

    /**
     * @brief set_connectionTimeoutMilliseconds Sets the number of milliseconds the
     * detector will wait for valid data from the serial port.
     * @param value
     */
    void set_connectionTimeoutMilliseconds(uint16_t value);

    /**
     * @brief trixterxdreamv1bikesettings Constructor, intializes from the default QSettings.
     */
    trixterxdreamv1settings();

    /**
     * @brief trixterxdreamv1bikesettings Constructor, initializes from the specified QSettings.
     * @param settings
     */
    trixterxdreamv1settings(const QSettings& settings);

    /**
     * @brief Load Loads the values from the default settings.
     */
    void Load();

    /**
     * @brief Load Loads the values from the specified QSettings object.
     * @param settings
     */
    void Load(const QSettings& settings);

    ///**
    // * @brief Save Saves the values to the default QSettings object.
    // */
    //void Save();

    ///**
    // * @brief Save Saves the values to the specified QSettings object.
    // * @param settings
    // */
    //void Save(const QSettings& settings);
};


#endif // TRIXTERXDREAMV1SETTINGS_H
