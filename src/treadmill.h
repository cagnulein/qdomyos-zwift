#ifndef TREADMILL_H
#define TREADMILL_H
#include "bluetoothdevice.h"
#include <QObject>

class treadmill : public bluetoothdevice {
    Q_OBJECT

  public:
    treadmill();
    void update_metrics(bool watt_calc, const double watts);
    metric lastRequestedSpeed() { return RequestedSpeed; }
    metric lastRequestedInclination() { return RequestedInclination; }
    virtual bool connected();
    virtual metric currentInclination();
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
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    void clearStats();
    void setLap();
    void setPaused(bool p);
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

  public slots:
    virtual void changeSpeed(double speed);
    virtual void changeInclination(double grade, double percentage);
    virtual void changeSpeedAndInclination(double speed, double inclination);
    virtual void cadenceSensor(uint8_t cadence);
    virtual void powerSensor(uint16_t power);
    virtual void speedSensor(double speed);
    virtual void instantaneousStrideLengthSensor(double length);
    virtual void groundContactSensor(double groundContact);
    virtual void verticalOscillationSensor(double verticalOscillation);

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


    void updateLockscreenStepCadence();

    void doPelotonWorkaround() override;
    void configureLockscreenFunctions(QZLockscreenFunctions * functions) const override;

};

#endif // TREADMILL_H
