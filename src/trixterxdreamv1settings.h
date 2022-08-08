#ifndef TRIXTERXDREAMV1SETTINGS_H
#define TRIXTERXDREAMV1SETTINGS_H


#include <cstdint>
#include "qmutex.h"
#include "qsettings.h"
#include "trixterxdreamv1client.h"


/**
 * @brief The trixterxdreamv1bikesettings class encapsulates the application settings for the Trixter X-Dream V1 Bike.
 * Field accessors restrict values to defined limits.
 */
class trixterxdreamv1settings {
public:
    // these should match the corresponding values in settings.qml
    // - the default values where the properties are defined
    // - the validations on the text boxes
    constexpr static bool DefaultEnabled =true;
    constexpr static bool DefaultSteeringEnabled =true;
    constexpr static bool DefaultHeartRateEnabled =true;
    constexpr static uint8_t DefaultSteeringCenter = trixterxdreamv1client::MaxSteering/2;
    constexpr static uint8_t DefaultSteeringDeadZoneWidth = 20;
    constexpr static uint8_t DefaultSteeringSensitivity = 100;
    constexpr static uint8_t MinSteeringCenter = DefaultSteeringCenter-60;
    constexpr static uint8_t MaxSteeringCenter = DefaultSteeringCenter+60;
    constexpr static uint8_t MaxSteeringDeadZoneWidth = 50;
    constexpr static uint8_t MinSteeringDeadZoneWidth = 0;
    constexpr static uint8_t MinSteeringSensitivityPercentage = 20;
    constexpr static uint8_t MaxSteeringSensitivityPercentage = 200;

    /**
     * @brief Defines QSettings keys relating to the Trixter X-Dream V1 bike.
     */
    class keys {
    public:
        /**
         * @brief Enabled QSettings key to specify if the Trixter X-Dream V1 Bike is enabled in the application.
         */
        const static QString Enabled;
        const static QString HeartRateEnabled;
        const static QString SteeringEnabled;
        const static QString SteeringCenter;
        const static QString SteeringDeadZoneWidth;
        const static QString SteeringSensitivityLeft;
        const static QString SteeringSensitivityRight;
    };

private:
    // mutex for thread syncing, may attempt double lock when loading from QSettings, so using recursive mutex
    QRecursiveMutex mutex;
    bool enabled=DefaultEnabled;
    bool steeringEnabled = DefaultSteeringEnabled;
    bool heartRateEnabled = DefaultHeartRateEnabled;
    uint8_t steeringCenter = DefaultSteeringCenter;
    uint8_t steeringDeadZoneWidth = DefaultSteeringDeadZoneWidth;
    uint8_t steeringSensitivityLeft = DefaultSteeringSensitivity;
    uint8_t steeringSensitivityRight = DefaultSteeringSensitivity;
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
     * @brief get_steeringCenter Gets the value considered to be the center position for the steering.
     * Defaults to half of trixterxdreamv1client::MaxSteering, but in reality is somewhat different
     * due to physical calibration.
     */
    uint8_t get_steeringCenter();

    /**
     * @brief set_steeringCenter Sets the steering center value. Used to accomodate the bike's calibration.
     * @param value The value, will be clipped to [MinSteeringCenter, MaxSteeringCenter].
     * @return The actual value set.
     */
    uint8_t set_steeringCenter(uint8_t value);

    /**
     * @brief get_steeringDeadZoneWidth Gets the width of the dead zone. This is the region
     * from the left to the right of steeringCenter for which the steering value will be mapped to 0 degrees.
     */
    uint8_t get_steeringDeadZoneWidth();

    /**
     * @brief set_steeringDeadZoneWidth Sets the width, left to right, of the "dead zone" surrounding the
     * steeringCenter value, for which the steering value will be mapped to 0 degrees.
     * @param value The width, left to right, of the dead zone. Clipped to [MinSteeringDeadZoneWidth, MaxSteeringDeadZoneWidth].
     * @return
     */
    uint8_t set_steeringDeadZoneWidth(uint8_t value);

    /**
     * @brief get_steeringSensitivityLeft Gets the sensitivity, as a percentage for how sensitive the
     * steering will be when turning left.
     * @return
     */
    uint8_t get_steeringSensitivityLeft();

    /**
     * @brief set_steeringSensitivityLeft Sets the sensitivity, as a percentage, for how sensitive the steering
     * will be when turning left.
     * @param value The value, a percentage clipped to [MinSteeringSensitivityPercentage, MaxSteeringSensitivityPercentage].
     * @return The actual value set.
     */
    uint8_t set_steeringSensitivityLeft(uint8_t value);

    /**
     * @brief get_steeringSensitivityLeft Gets the sensitivity, as a percentage for how sensitive the
     * steering will be when turning right. Valid range 20 to 200.
     * @return
     */
    uint8_t get_steeringSensitivityRight();


    /**
     * @brief set_steeringSensitivityRight Sets the sensitivity, as a percentage, for how sensitive the steering
     * will be when turning right.
     * @param value The value, a percentage clipped to [MinSteeringSensitivityPercentage, MaxSteeringSensitivityPercentage].
     * @return The actual value set.
     */
    uint8_t set_steeringSensitivityRight(uint8_t value);

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
