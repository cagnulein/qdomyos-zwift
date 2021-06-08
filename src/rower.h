#ifndef ROWER_H
#define ROWER_H

#include <QObject>
#include "bluetoothdevice.h"
#include "bike.h"

class rower:public bike
{
    Q_OBJECT

public:
    rower();
    metric lastRequestedResistance();
    metric lastRequestedPelotonResistance();
    metric lastRequestedCadence();
    metric lastRequestedPower();
    virtual metric currentResistance();
    virtual metric currentCadence();
    virtual metric currentStrokesCount();
    virtual metric currentStrokesLength();
    virtual QTime currentPace();
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

public slots:
    virtual void changeResistance(int8_t res);
    virtual void changeCadence(int16_t cad);
    virtual void changePower(int32_t power);
    virtual void changeRequestedPelotonResistance(int8_t resistance);
    virtual void cadenceSensor(uint8_t cadence);

signals:
    void bikeStarted();
    void resistanceChanged(int8_t resistance);
    void resistanceRead(int8_t resistance);

protected:
    metric Cadence;
    metric Resistance;
    metric RequestedResistance;
    metric RequestedPelotonResistance;
    metric RequestedCadence;
    metric RequestedPower;
    metric StrokesLength;
    metric StrokesCount;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double CrankRevs = 0;

    metric m_pelotonResistance;
};

#endif // ROWER_H
