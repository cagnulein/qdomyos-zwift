#ifndef ROWER_H
#define ROWER_H

#include "devices/bluetoothdevice.h"
#include <QObject>

class rower : public bluetoothdevice {
    Q_OBJECT

  public:
    rower();
    metric lastRequestedResistance();
    metric lastRequestedPelotonResistance();
    metric lastRequestedCadence();
    metric lastRequestedPower();
    metric lastRequestedSpeed() { return RequestedSpeed; }
    QTime lastRequestedPace();
    virtual QTime lastPace500m();
    metric currentResistance() override;
    virtual metric currentStrokesCount();
    virtual metric currentStrokesLength();
    QTime currentPace() override;
    QTime averagePace() override;
    QTime maxPace() override;
    virtual double requestedSpeed();
    uint8_t fanSpeed() override;
    double currentCrankRevolutions() override;
    uint16_t lastCrankEventTime() override;
    bool connected() override;
    virtual uint16_t watts();
    virtual resistance_t pelotonToBikeResistance(int pelotonResistance);
    virtual resistance_t resistanceFromPowerRequest(uint16_t power);
    bluetoothdevice::BLUETOOTH_TYPE deviceType() override;
    metric pelotonResistance();
    void clearStats() override;
    void setLap() override;
    void setPaused(bool p) override;
    QTime speedToPace(double Speed);
    void setGears(double d);
    double gears();    

  public slots:
    void changeResistance(resistance_t res) override;
    virtual void changeCadence(int16_t cad);
    void changePower(int32_t power) override;
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    void cadenceSensor(uint8_t cadence) override;
    void powerSensor(uint16_t power) override;
    virtual void changeSpeed(double speed);

  signals:
    void bikeStarted();
    void resistanceChanged(resistance_t resistance);
    void resistanceRead(resistance_t resistance);

  protected:
    metric Resistance;
    metric RequestedResistance;
    metric RequestedPelotonResistance;
    double requestInclination = -100;
    metric RequestedCadence;
    metric RequestedSpeed;
    volatile double requestSpeed = -1;
    metric StrokesLength;
    metric StrokesCount;
    uint16_t LastCrankEventTime = 0;
    resistance_t requestResistance = -1;
    double CrankRevs = 0;
    double m_gears = 0;
    resistance_t lastRawRequestedResistanceValue = -1;

    metric m_pelotonResistance;

    class rowerSpeedDistance {
      public:
        rowerSpeedDistance(double distance, double speed) {
            this->distance = distance;
            this->speed = speed;
        }
        double distance;
        double speed;
    };

    QList<rowerSpeedDistance *> speedLast500mValues;
};

#endif // ROWER_H
