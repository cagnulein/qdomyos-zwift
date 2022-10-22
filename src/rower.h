#ifndef ROWER_H
#define ROWER_H

#include "bike.h"
#include "bluetoothdevice.h"
#include <QObject>

class rower : public bluetoothdevice {
    Q_OBJECT

  public:
    rower();
    metric lastRequestedResistance();
    metric lastRequestedPelotonResistance();
    metric lastRequestedCadence();
    metric lastRequestedPower();
    virtual metric currentResistance();
    virtual metric currentStrokesCount();
    virtual metric currentStrokesLength();
    virtual QTime currentPace();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    virtual uint16_t watts();
    virtual resistance_t pelotonToBikeResistance(int pelotonResistance);
    virtual resistance_t resistanceFromPowerRequest(uint16_t power);
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    metric pelotonResistance();
    void clearStats();
    void setLap();
    void setPaused(bool p);

  public slots:
    virtual void changeResistance(resistance_t res);
    virtual void changeCadence(int16_t cad);
    virtual void changePower(int32_t power);
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    virtual void cadenceSensor(uint8_t cadence);
    virtual void powerSensor(uint16_t power);

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
    metric RequestedPower;
    metric StrokesLength;
    metric StrokesCount;
    uint16_t LastCrankEventTime = 0;
    resistance_t requestResistance = -1;
    double CrankRevs = 0;

    metric m_pelotonResistance;
};

#endif // ROWER_H
