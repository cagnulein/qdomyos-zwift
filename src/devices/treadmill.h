#ifndef TREADMILL_H
#define TREADMILL_H
#include "devices/bluetoothdevice.h"
#include <QObject>
#include "treadmillErgTable.h"

class treadmill : public bluetoothdevice {
    Q_OBJECT

  public:
    treadmill();
    void update_metrics(bool watt_calc, const double watts, const bool from_accessory = false);
    metric lastRequestedSpeed() { return RequestedSpeed; }
    QTime lastRequestedPace();
    metric lastRequestedInclination() { return RequestedInclination; }
    bool connected() override;
    metric currentInclination() override;
    virtual double requestedSpeed();
    virtual double currentTargetSpeed();
    virtual double requestedInclination();
    metric lastRequestedPower();
    virtual double minStepInclination();
    virtual double minStepSpeed();
    virtual bool canStartStop() { return true; }
    metric currentStrideLength() { return InstantaneousStrideLengthCM; }
    metric currentGroundContact() { return GroundContactMS; }
    metric currentVerticalOscillation() { return VerticalOscillationMM; }
    metric currentStepCount() { return StepCount; }
    virtual uint16_t watts(double weight);
    static uint16_t wattsCalc(double weight, double speed, double inclination);
    BLUETOOTH_TYPE deviceType() override;
    void clearStats() override;
    void setLap() override;
    void setPaused(bool p) override;
    double lastRawSpeedRequested() {
        return (m_lastRawSpeedRequested != -1 ? m_lastRawSpeedRequested : currentSpeed().value());
    }
    double lastRawInclinationRequested() {
        return (m_lastRawInclinationRequested != -100 ? m_lastRawInclinationRequested : currentInclination().value());
    }
    virtual void setLastSpeed(double speed);
    virtual void setLastInclination(double inclination);
    virtual bool autoPauseWhenSpeedIsZero();
    virtual bool autoStartWhenSpeedIsGreaterThenZero();
    static double treadmillInclinationOverride(double Inclination);
    static double treadmillInclinationOverrideReverse(double Inclination);
    void cadenceFromAppleWatch();
    double calculateCadenceFromSpeed(double speed);
    virtual bool canHandleSpeedChange() { return true; }
    virtual bool canHandleInclineChange() { return true; }
    double runningStressScore();
    QTime speedToPace(double Speed);

  public slots:
    virtual void changeSpeed(double speed);
    void changeInclination(double grade, double percentage) override;
    void changePower(int32_t power) override;
    virtual void changeSpeedAndInclination(double speed, double inclination);
    void cadenceSensor(uint8_t cadence) override;
    void powerSensor(uint16_t power) override;
    void speedSensor(double speed) override;
    void inclinationSensor(double grade, double inclination) override;
    void instantaneousStrideLengthSensor(double length) override;
    void groundContactSensor(double groundContact) override;
    void verticalOscillationSensor(double verticalOscillation) override;

  signals:
    void tapeStarted();

  protected:
    volatile double requestSpeed = -1;
    double targetSpeed = -1;
    double requestInclination = -100;
    double lastSpeed = 0.0;
    double lastInclination = 0;
    metric rawSpeed;
    metric rawInclination;
    metric RequestedSpeed;
    metric RequestedInclination;
    metric InstantaneousStrideLengthCM;
    metric GroundContactMS;
    metric VerticalOscillationMM;    
    double m_lastRawSpeedRequested = -1;
    double m_lastRawInclinationRequested = -100;
    bool instantaneousStrideLengthCMAvailableFromDevice = false;
    treadmillErgTable _ergTable;
    
    // Power following logic
    bool callingFromFollowPower = false;  // Flag to track if change comes from followPowerBySpeed
    double targetWatts = -1;              // Target watts to maintain during power following

    void parseSpeed(double speed);
    void parseInclination(double speed);
    bool areInclinationSettingsDefault();

  private:
    bool simulateInclinationWithSpeed();
    bool followPowerBySpeed();
    void evaluateStepCount();
};

#endif // TREADMILL_H
