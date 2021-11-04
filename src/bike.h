#ifndef BIKE_H
#define BIKE_H

#include "bluetoothdevice.h"
#include <QObject>

class bike : public bluetoothdevice {

    Q_OBJECT

  public:
    bike();
    metric lastRequestedResistance();
    metric lastRequestedPelotonResistance();
    metric lastRequestedCadence();
    metric lastRequestedPower();
    virtual metric currentResistance();
    virtual metric currentCadence();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    virtual uint16_t watts();
    virtual int pelotonToBikeResistance(int pelotonResistance);
    virtual uint8_t resistanceFromPowerRequest(uint16_t power);
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    metric pelotonResistance();
    void clearStats();
    void setLap();
    void setPaused(bool p);
    uint8_t metrics_override_heartrate();
    void setGears(int8_t d);
    int8_t gears();

  public Q_SLOTS:
    virtual void changeResistance(int8_t res);
    virtual void changeCadence(int16_t cad);
    virtual void changePower(int32_t power);
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    virtual void cadenceSensor(uint8_t cadence);
    virtual void powerSensor(uint16_t power);
    virtual void changeInclination(double grade, double percentage);

  Q_SIGNALS:
    void bikeStarted();
    void resistanceChanged(int8_t resistance);
    void resistanceRead(int8_t resistance);

  protected:
    metric RequestedResistance;
    metric RequestedPelotonResistance;
    metric RequestedCadence;
    metric RequestedPower;

    int8_t requestResistance = -1;
    double requestInclination = -1;
    int16_t requestPower = -1;

    int8_t m_gears = 0;
    int8_t lastRawRequestedResistanceValue = -1;
    uint16_t LastCrankEventTime = 0;
    double CrankRevs = 0;

    metric m_pelotonResistance;
};

#endif // BIKE_H
