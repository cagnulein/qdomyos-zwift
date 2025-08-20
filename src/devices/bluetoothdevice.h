#ifndef BLUETOOTHDEVICE_H
#define BLUETOOTHDEVICE_H

#include "definitions.h"
#include "metric.h"
#include "qzsettings.h"
#include "ergtable.h"

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QObject>
#include <QTimer>

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>

#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>

#include "virtualdevices/virtualdevice.h"

#if defined(Q_OS_IOS)
#define SAME_BLUETOOTH_DEVICE(d1, d2) (d1.deviceUuid() == d2.deviceUuid())
#else
#define SAME_BLUETOOTH_DEVICE(d1, d2) (d1.address() == d2.address())
#endif

/**
 * @brief The MetersByInclination class represents a section of track at a specific inclination.
 */
class MetersByInclination {
  public:
    /**
     * @brief meters The length of the section. Units: meters
     */
    double meters;

    /**
     * @brief inclination The inclination of the section.
     * Units: Percentage vertical to horizontal
     */
    double inclination;
};

class bluetoothdevice : public QObject {

    Q_OBJECT

  public:
    bluetoothdevice();

    ~bluetoothdevice() override;

    /**
     * @brief currentHeart Gets a metric object for getting and setting the current heart rate. Units: beats per minute
     */
    virtual metric currentHeart();

    /**
     * @brief currentSpeed Gets a metric object for getting and setting the speed. Units: km/h
     */
    virtual metric currentSpeed();

    /**
     * @brief currentPace Gets the current pace. Units: time per km
     */
    virtual QTime currentPace();

    /**
     * @brief currentInclination The current inclination.
     * Units: Percentage vertical to horizontal
     * Expected range: Depends on device.
     */
    virtual metric currentInclination();

    /**
     * @brief setInclination Set the protected Inclination metric, which could be different from that
     * returned by an overridden currentInclination().
     * @param inclination The inclination. Units: Percentage vertical to horizontal. Expected range: Depends on device.
     */
    void setInclination(double inclination);

    /**
     * @brief averagePace Gets the average time per kilometer travelled. Units: time per km
     */
    virtual QTime averagePace();

    /**
     * @brief maxPace Gets the maximum pace (minimum time per kilometer) for the session. Units: time per km
     */
    virtual QTime maxPace();

    /**
     * @brief odometer Gets the total distance travelled. Units: km
     * @return
     */
    virtual double odometer();
    virtual double odometerFromStartup();
    virtual metric currentDistance() {return Distance;}
    virtual metric currentDistance1s() {return Distance1s;}
    void addCurrentDistance1s(double distance) { Distance1s += distance; }

    /**
     * @brief calories Gets a metric object to get and set the amount of energy expended.
     * Default implementation returns the protected KCal property, potentially adjusted for active calories. Units: kcal
     * Other implementations could have different units.
     * @return
     */
    virtual metric calories();

    /**
     * @brief totalCalories Gets total calories (including BMR) regardless of active calories setting.
     * @return Total calories metric
     */
    virtual metric totalCalories();

    /**
     * @brief jouls Gets a metric object to get and set the number of joules expended. Units: joules
     */
    metric jouls();

    /**
     * @brief fanSpeed Gets the current fan speed. Units: depends on device
     */
    virtual uint8_t fanSpeed();

    /**
     * @brief elapsedTime The elapsed time for the session(?).
     */
    virtual QTime elapsedTime();

    /**
     * @brief offsetElapsedTime Shifts the elapsed time (stored in the protected member: elapsed)
     * for the session by the specified offset.
     * @param offset The time offset to shift by. Default unit: seconds but this could be overridden.
     */
    virtual void offsetElapsedTime(int offset);

    /**
     * @brief movingTime Gets the time spent moving.
     */
    virtual QTime movingTime();

    /**
     * @brief lapElapsedTime Gets the time elapsed on the current lap.
     */
    virtual QTime lapElapsedTime();

    /**
     * @brief lapOdometer Gets the distance elapsed on the current lap.
     */
    virtual double lapOdometer();

    /**
     * @brief connected Gets a value to indicate if the device is connected.
     */
    virtual bool connected();

    /**
     * @brief currentResistance Gets a metric object to get or set the currently requested resistance.
     * Expected range: 0 to maxResistance()
     */
    virtual metric currentResistance();

    /**
     * @brief currentCadence Gets a metric object to get and set the current cadence. Units: revolutions per minute
     */
    virtual metric currentCadence();

    /**
     * @brief currentCrankRevolutions Gets the current total number of crank revolutions.
     */
    virtual double currentCrankRevolutions();

    /**
     * @brief currentCordinate Gets the current geographic coordinates.
     */
    virtual QGeoCoordinate currentCordinate();

    /**
     * @brief nextInclination300Meters The next 300m of track sections: length and inclination
     * @return A list of MetersByInclination objects
     */
    virtual QList<MetersByInclination> nextInclination300Meters() { return NextInclination300Meters; }

    /**
     * @brief currentAzimuth Gets the current azimuth. Units: degrees (? = North)
     */
    virtual double currentAzimuth() { return azimuth; }

    /**
     * @brief averageAzimuthNext300m Gets the average azimuth for the next 300m
     * Units: degrees (? = North)
     */
    virtual double averageAzimuthNext300m() { return azimuthAvgNext300m; }

    /**
     * @brief setAverageAzimuthNext300m Sets the average azimuth for the next 300m.
     * @param azimuth The azimuth. Units: degrees (? = North)
     */
    virtual void setAverageAzimuthNext300m(double azimuth) { azimuthAvgNext300m = azimuth; }

    /**
     * @brief lastCrankEventTime The time of the last crank event. Units: 1/1024s
     */
    virtual uint16_t lastCrankEventTime();

    /**
     * @brief VirtualDevice The virtual bridge to Zwift for example, or to any 3rd party app.
     */
    virtualdevice *VirtualDevice();

    /**
     * @brief watts Calculates the amount of power used. Units: watts
     * @param weight The weight of the rider. Units: kg
     */
    uint16_t watts(double weight);

    /**
     * @brief wattsMetric Gets a metric object to get or set the amount of power used.  Units: watts
     */
    metric wattsMetric();

    /**
     * @brief wattsMetricforUi Show the wattage applying averaging in case the user requested this.  Units: watts
     */
    double wattsMetricforUI() {
        QSettings settings;
        bool power5s = settings.value(QZSettings::power_avg_5s, QZSettings::default_power_avg_5s).toBool();
        if (power5s)
            return wattsMetric().average5s();
        else
            return wattsMetric().value();
    }

    /**
     * @brief changeFanSpeed Tries to change the fan speed.
     * @param speed The requested fan speed. Units: depends on device
     */
    virtual bool changeFanSpeed(uint8_t speed);

    /**
     * @brief elevationGain Gets a metric object to get and set the elevation gain. Units: ?
     */
    virtual metric elevationGain();

    /**
     * @brief clearStats Clear the statistics.
     */
    virtual void clearStats();

    /**
     * @brief bluetoothDevice The bluetooth device information.
     */
    QBluetoothDeviceInfo bluetoothDevice;

    /**
     * @brief disconnectBluetooth Disconnect from the device from bluetooth.
     */
    void disconnectBluetooth();

    /**
     * @brief setPaused Sets the paused mode.
     * @param p True to pause, false to resume.
     */
    virtual void setPaused(bool p);

    /**
     * @brief isPaused Indicates if the device is currently paused.
     */
    bool isPaused() { return paused; }

    /**
     * @brief setLap Begins a new lap for the statistics calculated by the metrics objects.
     */
    virtual void setLap();

    /**
     * @brief setAutoResistance Toggles auto-resistance.
     */
    void setAutoResistance(bool value) { autoResistanceEnable = value; }

    /**
     * @brief autoResistance Indicates the state of auto-resistance.
     * @return
     */
    bool autoResistance() { return autoResistanceEnable; }

    /**
     * @brief setDifficult Sets the difficulty gain level.
     * @param d The difficulty level. Units: depends on implementation.
     */
    void setDifficult(double d);

    /**
     * @brief difficult Gets the difficulty gain level. Units: depends on implementation.
     * @return
     */
    double difficult();

    /**
     * @brief setDifficult Sets the difficulty gain level.
     * @param d The difficulty level. Units: depends on implementation.
     */
    void setInclinationDifficult(double d);

    /**
     * @brief difficult Gets the difficulty gain level. Units: depends on implementation.
     * @return
     */
    double inclinationDifficult();

    /**
     * @brief setDifficult Sets the difficulty offset level.
     * @param d The difficulty level. Units: depends on implementation.
     */
    void setDifficultOffset(double d);

    /**
     * @brief difficult Gets the difficulty offset level. Units: depends on implementation.
     * @return
     */
    double difficultOffset();

    /**
     * @brief setDifficult Sets the difficulty offset level.
     * @param d The difficulty level. Units: depends on implementation.
     */
    void setInclinationDifficultOffset(double d);

    /**
     * @brief difficult Gets the difficulty offset level. Units: depends on implementation.
     * @return
     */
    double inclinationDifficultOffset();

    /**
     * @brief weightLoss Gets the value of the weight loss metric. Units: kg
     * @return
     */
    double weightLoss() { return WeightLoss.value(); }

    /**
     * @brief wattKg Gets a metric object to get and set the watt kg of something. Units: watt kg
     * @return
     */
    metric wattKg() { return WattKg; }

    /**
     * @brief currentMETS Gets a metric object to get and set the current METS (Metabolic Equivalent of Tasks)
     * Units: METs (1 MET is approximately 3.5mL of Oxygen consumed per kg of body weight per minute)
     */
    metric currentMETS() { return METS; }

    /**
     * @brief currentHeartZone Gets a metric object to get or set the current heart zone. Units: depends on
     * implementation.
     */
    metric currentHeartZone() { return HeartZone; }

    /*
    * @brief maxHeartZone Gets the maximum number of heart zones.
    */
    uint8_t maxHeartZone() { return maxhrzone; }

    /**
     * @brief secondsForHeartZone Gets the number of seconds in the current heart zone.
     * 
     * @param zone The heart zone.
     */
    uint32_t secondsForHeartZone(uint8_t zone);

    /**
     * @brief currentHeatZone Gets a metric object to get or set the current heat zone. Units: depends on
     * implementation (based on Heat Strain Index: Zone 1: 0-1.99, Zone 2: 2-2.99, Zone 3: 3-6.99, Zone 4: 7+)
     */
    metric currentHeatZone() { return HeatZone; }

    /**
     * @brief maxHeatZone Gets the maximum number of heat zones.
     */
    uint8_t maxHeatZone() { return maxheatzone; }

    /**
     * @brief secondsForHeatZone Gets the number of seconds in the current heat zone.
     * 
     * @param zone The heat zone.
     */
    uint32_t secondsForHeatZone(uint8_t zone);

    /**
     * @brief currentPowerZone Gets a metric object to get or set the current power zome. Units: depends on
     * implementation.
     * @return
     */
    metric currentPowerZone() { return PowerZone; }

    /**
     * @brief currentPowerZone Gets a metric object to get or set the current power zome. Units: depends on
     * implementation.
     * @return
     */
    metric targetPowerZone() { return TargetPowerZone; }

    /**
     * @brief setGPXFile Sets the file for GPS data exchange.
     * @param filename The file path.
     */
    void setGPXFile(QString filename);

    /**
     * @brief currentGPXBase64 Returns the Base64 encode for the current GPX used.
     */
    QString currentGPXBase64() { return gpxBase64; }

    // in the future these 2 should be calculated inside the update_metrics()

    /**
     * @brief setHeartZone Set the current heart zone.
     * This is equivalent to currentHeartZone().setvalue(hz)
     * @param hz The heart zone. Unit: depends on implementation.
     */
    void setHeartZone(double hz);

    /**
     * @brief setHeatZone Set the current heat zone based on Heat Strain Index.
     * This is equivalent to currentHeatZone().setvalue(hz)
     * @param heatStrainIndex The heat strain index to determine zone. Unit: depends on implementation.
     */
    void setHeatZone(double heatStrainIndex);

    /**
     * @brief setPowerZone Set the current power zone.
     * This is equivalent to currentPowerZone().setvalue(pz)
     * @param pz The power zone. Unit: depends on implementation.
     */
    void setPowerZone(double pz) { PowerZone = pz; }

    /**
     * @brief setTargetPowerZone Set the target power zone.
     * This is equivalent to targetPowerZone().setvalue(pz)
     * @param pz The power zone. Unit: depends on implementation.
     */
    void setTargetPowerZone(double pz) { TargetPowerZone = pz; }

    enum BLUETOOTH_TYPE { UNKNOWN = 0, TREADMILL, BIKE, ROWING, ELLIPTICAL, JUMPROPE, STAIRCLIMBER };
    enum WORKOUT_EVENT_STATE { STARTED = 0, PAUSED = 1, RESUMED = 2, STOPPED = 3 };

    /**
     * @brief deviceType The type of device represented by this object.
     */
    virtual BLUETOOTH_TYPE deviceType();

    /**
     * @brief metrics Gets a list of available metrics.
     * @return
     */
    static QStringList metrics();

    /**
     * @brief metrics_override_heartrate Provides a way to override the metrics heart rate with another metric.
     * Units: beats per minute
     */
    virtual uint8_t metrics_override_heartrate();

    /**
     * @brief Overridden in subclasses to specify the maximum resistance level supported by the device.
     */
    virtual resistance_t maxResistance();

    // Metrics for core temperature data
    metric CoreBodyTemperature;  // Core body temperature in °C or °F
    metric SkinTemperature;      // Skin temperature in °C or °F
    metric HeatStrainIndex;      // Heat Strain Index (0-25.4, scaled by 10)

  public Q_SLOTS:
    virtual void start();
    virtual void stop(bool pause);
    virtual void heartRate(uint8_t heart);
    virtual void cadenceSensor(uint8_t cadence);
    virtual void powerSensor(uint16_t power);
    virtual void speedSensor(double speed);
    virtual void coreBodyTemperature(double coreBodyTemperature);
    virtual void skinTemperature(double skinTemperature);
    virtual void heatStrainIndex(double heatStrainIndex);
    virtual void inclinationSensor(double grade, double inclination);
    virtual void changeResistance(resistance_t res);
    virtual void changePower(int32_t power);
    virtual void changeInclination(double grade, double percentage);
    virtual void changeGeoPosition(QGeoCoordinate p, double azimuth, double avgAzimuthNext300Meters);
    virtual void workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state);
    virtual void instantaneousStrideLengthSensor(double length);
    virtual void groundContactSensor(double groundContact);
    virtual void verticalOscillationSensor(double verticalOscillation);
    virtual void changeNextInclination300Meters(QList<MetersByInclination> i) { NextInclination300Meters = i; }

  Q_SIGNALS:
    void connectedAndDiscovered();
    void speedChanged(double speed);
    void cadenceChanged(uint8_t cadence);
    void powerChanged(uint16_t power);
    void inclinationChanged(double grade, double percentage);
    void fanSpeedChanged(uint8_t speed);
    void instantaneousStrideLengthChanged(double length);
    void groundContactChanged(double groundContact);
    void verticalOscillationChanged(double verticalOscillation);

  protected:
    /**
     * @brief Mode of operation for the virtual device with the bluetoothdevice object.
     */
    enum VIRTUAL_DEVICE_MODE {

        /**
         * @brief Not set.
         */
        NONE,
        /**
         * @brief Virtual device represents the same type of device.
         */
        PRIMARY,

        /**
         * @brief Virtual device representing the device for a purpose other than the
         * type of device it matches.
         */
        ALTERNATIVE
    };

    /**
     * @brief hasVirtualDevice shows if the object has any virtual device assigned.
     */
    bool hasVirtualDevice();

    QLowEnergyController *m_control = nullptr;

    /**
     * @brief elapsed A metric object to get and set the elapsed time for the session. Units: seconds
     */
    metric elapsed;

    /**
     * @brief moving The time spent moving in the current session. Units: seconds
     */
    metric moving; // moving time

    /**
     * @brief KCal The number of kilocalories expended in the session. Units: kcal
     */
    metric KCal;

    /**
     * @brief Speed The simulated speed of the device. Units: km/h
     * e.g. the product of bike flywheel speed and simulated wheel size, or
     * the belt speed of a treadmill.
     */
    metric Speed;

    /**
     * @brief Distance The simulated distance travelled. Units: km
     * Depends on the device.
     * e.g. the number of bike flywheel revolutions multiplied by the simulated wheel circumference, or
     *      the length of belt traversed on a treadmill.
     */
    metric Distance;
    metric Distance1s; // used to populate the distance on the FIT file. Since Strava is using the distance to graph it, it has to have 1s trigger.

    /**
     * @brief FanSpeed The currently requested fan speed. Units: revolutions per second
     */
    uint8_t FanSpeed = 0;

    /**
     * @brief Heart rate. Unit: beats per minute
     */
    metric Heart;

    int8_t requestStart = -1;
    int8_t requestStop = -1;
    int8_t requestPause = -1;
    int8_t requestIncreaseFan = -1;
    int8_t requestDecreaseFan = -1;
    double requestFanSpeed = -1;

    int64_t lastStart = 0;
    int64_t lastStop = 0;

    metric RequestedPower;
    int16_t requestPower = -1;

    /**
     * @brief m_difficult The current difficulty gain. Units: device dependent
     */
    double m_difficult = 1.0;

    /**
     * @brief m_difficult The current difficulty gain. Units: device dependent
     */
    double m_inclination_difficult = 1.0;

    /**
     * @brief m_difficult The current difficulty offset. Units: device dependent
     */
    double m_difficult_offset = 0.0;

    /**
     * @brief m_difficult The current difficulty offset. Units: device dependent
     */
    double m_inclination_difficult_offset = 0.0;

    /**
     * @brief m_jouls The number of joules expended in the current session. Unit: joules
     */
    metric m_jouls;

    /**
     * @brief elevationAcc The elevation gain. Units: meters
     */
    metric elevationAcc;

    /**
     * @brief m_watt Metric to get and set the power read from the trainer or from the power sensor Unit: watts
     */
    metric m_watt;
    
    /**
     * @brief m_rawWatt Metric to get and set the power from the trainer only. Unit: watts
     */
    metric m_rawWatt;

    /**
     * @brief WattKg Metric to get and set the watt kg for the session (what's this?). Unit: watt kg
     */
    metric WattKg;

    /**
     * @brief WeightLoss Metric to get and set the lost weight in the session (?). Unit: kg
     */
    metric WeightLoss;

    /**
     * @brief The speed at which the crank is turning. Units: device-specific actions per minute
     * e.g. crank revolutions on a bike, steps on a treadmill,
     * strokes on a rower, stride rate on an elliptical trainer
     */
    metric Cadence;

    /**
     * @brief The currently requested resistance level. Expected range: 0 to maxResistance()
     */
    metric Resistance;

    /**
     * @brief METS A metric object to get and set the METS (Metabolic Equivalent of Tasks) for the session.
     * Units: METs (1 MET is approximately 3.5mL of Oxygen consumed per kg of body weight per minute)
     */
    metric METS;

    /**
     * @brief coordinate The geolocation for the device.
     */
    QGeoCoordinate coordinate;

    /**
     * @brief azimuth The azimuth. Units: degrees (? = North)
     */
    double azimuth;

    /**
     * @brief azimuthAvgNext300m The average azimuth for the next 300m. Units: degrees (? = North)
     */
    double azimuthAvgNext300m;

    /**
     * @brief coordinateTS ???. Unit: ???
     */
    quint64 coordinateTS = 0;

    /**
     * @brief coordinateOdometer ???. Unit: ???
     */
    double coordinateOdometer = 0;

    /**
     * @brief The currently loaded gpxBase64 GPS exchange data.
     */
    QString gpxBase64 = "";

    /**
     * @brief gpxFileName The file path of the currently loaded GPS exchange data.
     */
    QString gpxFileName = "";

    /**
     * @brief NextInclination300Meters A list of the length and inclination of track sections for the next 300m
     */
    QList<MetersByInclination> NextInclination300Meters;

    /**
     * @brief Inclination A metric to get and set the currently requested inclinaton. Units: degrees (0 = horizontal)
     */
    metric Inclination;

    /**
     * @brief HeartZone A metric to get and set the current heart zone. Unit: depends on implementation
     */
    metric HeartZone;

    /**
     * @brief HeatZone A metric to get and set the current heat zone. Unit: depends on implementation
     */
    metric HeatZone;

    /**
     * @brief PowerZone A metric to get and set the current power zone. Unit: depends on implementation
     */
    metric PowerZone;

    /**
     * @brief TargetPowerZone A metric to get and set the target power zone. Unit: depends on implementation
     */
    metric TargetPowerZone;

    /**
     * @brief _ergTable The current erg table
     */
    ergTable _ergTable;
    
    /**
     * @brief StepCount A metric to get and set the step count. Unit: step
     */
    metric StepCount;

    /**
     * @brief Collect the number of seconds in each zone for the current heart rate
     */
    static const uint8_t maxhrzone = 5;
    metric hrZonesSeconds[maxhrzone];

    /**
     * @brief Collect the number of seconds in each zone for the current heat strain index
     */
    static const uint8_t maxheatzone = 4;
    metric heatZonesSeconds[maxheatzone];

    bluetoothdevice::WORKOUT_EVENT_STATE lastState;

    /**
     * @brief paused Indicates if the device is currently paused.
     */
    bool paused = false;

    /**
     * @brief autoResistanceEnable Indicates if auto-resistance is currently enabled.
     */
    bool autoResistanceEnable = true;

    /**
     * @brief _lastTimeUpdate The time the (client was last updated / last update was received from the device) ???
     */
    QDateTime _lastTimeUpdate;

    /**
     * @brief _firstUpdate Indicates if this is the first update.
     */
    bool _firstUpdate = true;

    /**
     * @brief update_metrics Updates the metrics given the specified inputs.
     * @param watt_calc ??
     * @param watts ?. Unit: watts
     */
    void update_metrics(bool watt_calc, const double watts, const bool from_accessory = false);

    /**
     * @brief update_hr_from_external Updates heart rate from Garmin Companion App or Apple Watch
     */
    void update_hr_from_external();

    /**
     * @brief calculateMETS Calculate the METS (Metabolic Equivalent of Tasks)
     * Units: METs (1 MET is approximately 3.5mL of Oxygen consumed per kg of body weight per minute)
     */
    double calculateMETS();

    /**
     * @brief setVirtualDevice Set the virtual device, and the way it is being used. Deletes the existing one, if
     * present.
     * @param virtualDevice The virtual device.
     */
    void setVirtualDevice(virtualdevice *virtualDevice, VIRTUAL_DEVICE_MODE mode);

    /**
     * @brief writeBuffer contains the last byte array request via bluetooth to the fitness devices
     */
    QByteArray *writeBuffer = nullptr;

  private:
    /**
     * @brief Indicates the way the virtual device is being used.
     * Normally PRIMARY, set this to ALTERNATIVE where the device is being used unusually, e.g.
     * for the Zwift Auto-Inclination Workaround.
     */
    VIRTUAL_DEVICE_MODE virtualDeviceMode = VIRTUAL_DEVICE_MODE::NONE;
    virtualdevice *virtualDevice = nullptr;

  protected:
    // useful to understand if a power sensor device for treadmill, it's a real one like the stryd or it's a dumb one like the runpod from Zwift
    bool powerReceivedFromPowerSensor = false;
};

#endif // BLUETOOTHDEVICE_H
