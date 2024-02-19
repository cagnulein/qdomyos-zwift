#ifndef TREADMILL_H
#define TREADMILL_H
#include "devices/bluetoothdevice.h"
#include <QObject>

class treadmill : public bluetoothdevice {
    Q_OBJECT

  public:
    treadmill();
    void update_metrics(bool watt_calc, const double watts);
    metric lastRequestedSpeed() { return RequestedSpeed; }
    QTime lastRequestedPace();
    metric lastRequestedInclination() { return RequestedInclination; }
    bool connected() override;
    metric currentInclination() override;
    virtual double requestedSpeed();
    virtual double currentTargetSpeed();
    virtual double requestedInclination();
    virtual double minStepInclination();
    virtual double minStepSpeed();
    virtual bool canStartStop() { return true; }
    metric currentStrideLength() { return InstantaneousStrideLengthCM; }
    metric currentGroundContact() { return GroundContactMS; }
    metric currentVerticalOscillation() { return VerticalOscillationMM; }
    metric currentStepCount() { return StepCount; }
    uint16_t watts(double weight);
    static uint16_t wattsCalc(double weight, double speed, double inclination);
    bluetoothdevice::BLUETOOTH_TYPE deviceType() override;
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
    virtual bool canHandleSpeedChange() { return true; }
    virtual bool canHandleInclineChange() { return true; }

  public slots:
    virtual void changeSpeed(double speed);
    void changeInclination(double grade, double percentage) override;
    virtual void changeSpeedAndInclination(double speed, double inclination);
    void cadenceSensor(uint8_t cadence) override;
    void powerSensor(uint16_t power) override;
    void speedSensor(double speed) override;
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
    metric RequestedSpeed;
    metric RequestedInclination;
    metric InstantaneousStrideLengthCM;
    metric GroundContactMS;
    metric VerticalOscillationMM;
    metric StepCount;
    double m_lastRawSpeedRequested = -1;
    double m_lastRawInclinationRequested = -100;
    bool instantaneousStrideLengthCMAvailableFromDevice = false;

    void parseSpeed(double speed);

  private:
    bool simulateInclinationWithSpeed();
    void evaluateStepCount();
};

#endif // TREADMILL_H
