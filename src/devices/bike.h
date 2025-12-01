#ifndef BIKE_H
#define BIKE_H

#include "devices/bluetoothdevice.h"
#include "virtualdevices/virtualbike.h"
#include <QObject>
#include <QElapsedTimer>

class bike : public bluetoothdevice {

    Q_OBJECT

  public:
    bike();

    virtualbike *VirtualBike();

    metric lastRequestedResistance();
    metric lastRequestedPelotonResistance();
    metric lastRequestedCadence();
    metric lastRequestedPower();
    metric currentResistance() override;
    uint8_t fanSpeed() override;
    double currentCrankRevolutions() override;
    uint16_t lastCrankEventTime() override;
    bool connected() override;
    double defaultMaxGears() { return 9999.0; }
    virtual double maxGears() { return defaultMaxGears(); }
    virtual double minGears() { return -9999.0; }
    virtual uint16_t watts();
    virtual resistance_t pelotonToBikeResistance(int pelotonResistance);
    virtual resistance_t resistanceFromPowerRequest(uint16_t power);
    virtual uint16_t powerFromResistanceRequest(resistance_t requestResistance);
    virtual bool ergManagedBySS2K() { return false; }
    BLUETOOTH_TYPE deviceType() override;
    metric pelotonResistance();
    void clearStats() override;
    void setLap() override;
    void setPaused(bool p) override;
    uint8_t metrics_override_heartrate() override;
    void setGears(double d);
    double gears();
    double gearsZwiftRatio();
    void setSpeedLimit(double speed) { m_speedLimit = speed; }
    double speedLimit() { return m_speedLimit; }
    virtual bool ifitCompatible() {return false;}

    /**
     * @brief currentSteeringAngle Gets a metric object to get or set the current steering angle
     * for the Elite Sterzo or emulating device. Expected range -45 to +45 degrees.
     * @return A metric object.
     */
    metric currentSteeringAngle() { return m_steeringAngle; }
    virtual bool inclinationAvailableByHardware();
    virtual bool inclinationAvailableBySoftware();
    bool ergModeSupportedAvailableByHardware() { return ergModeSupported; }
    virtual bool ergModeSupportedAvailableBySoftware() { return ergModeSupported; }

  public Q_SLOTS:
    void changeResistance(resistance_t res) override;
    virtual void changeCadence(int16_t cad);
    void changePower(int32_t power) override;
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    void cadenceSensor(uint8_t cadence) override;
    void powerSensor(uint16_t power) override;
    void changeInclination(double grade, double percentage) override;
    virtual void changeSteeringAngle(double angle) { m_steeringAngle = angle; }
    virtual void resistanceFromFTMSAccessory(resistance_t res) { Q_UNUSED(res); }
    void gearUp() {
        QSettings settings;
        bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();
        setGears(gears() + (gears_zwift_ratio ? 1 :
                                settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble()));
    }
    void gearDown() {
        QSettings settings;
        bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();
        setGears(gears() - (gears_zwift_ratio ? 1 :
                                settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble()));
    }

  Q_SIGNALS:
    void bikeStarted();
    void resistanceChanged(resistance_t resistance);
    void resistanceRead(resistance_t resistance);
    void steeringAngleChanged(double angle);
    void gearOkUp(); // Signal when gear up succeeds
    void gearOkDown(); // Signal when gear down succeeds
    void gearFailedUp();   // Signal when gear up hits max
    void gearFailedDown(); // Signal when gear down hits min

  protected:
    metric RequestedResistance;
    metric RequestedPelotonResistance;
    metric RequestedCadence;

    resistance_t requestResistance = -1;
    double requestInclination = -100;

    bool ergModeSupported = false; // if a bike has this mode supported, when from the virtual bike there is a power
                                   // request there is no need to translate in resistance levels

    double m_gears = 0;
    resistance_t lastRawRequestedResistanceValue = -1;
    double lastRawRequestedInclinationValue = -100;
    uint16_t LastCrankEventTime = 0;
    double CrankRevs = 0;

    metric m_pelotonResistance;

    metric m_steeringAngle;

    double m_speedLimit = 0;

    // Sim mode support: convert inclination to power for devices without native inclination
    bool m_slopeControlEnabled = false;
    double m_currentSlopePercent = 0.0;
    int m_lastSlopeTargetPower = -1;
    bool m_slopePowerChangeInProgress = false;
    QElapsedTimer m_slopePowerTimer;

    // Physics-based power calculation from slope
    virtual double computeSlopeTargetPower(double gradePercent, double speedKmh);

    // Update power based on current slope and speed (called periodically)
    virtual void updateSlopeTargetPower(bool force = false);

    // Check if device supports native inclination control
    virtual bool supportsNativeInclination() const { return true; }

    // Helper: get current speed for slope calculations
    double getCurrentSpeedForSlope();

    uint16_t wattFromHR(bool useSpeedAndCadence);
};

#endif // BIKE_H
