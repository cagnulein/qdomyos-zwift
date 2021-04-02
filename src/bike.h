#ifndef BIKE_H
#define BIKE_H

#include <QObject>
#include "bluetoothdevice.h"

class bike:public bluetoothdevice
{
    Q_OBJECT

public:
    bike();
    metric lastRequestedResistance();
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
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    metric pelotonResistance();
    void clearStats();
    void setLap();
    void setPaused(bool p);

public slots:
    virtual void changeResistance(int8_t res);
    virtual void changeCadence(int16_t cad);
    virtual void changePower(int32_t power);

signals:
    void bikeStarted();

protected:
    metric Cadence;
    metric Resistance;
    metric RequestedResistance;    
    metric RequestedCadence;
    metric RequestedPower;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double CrankRevs = 0;

    metric m_pelotonResistance;
};

#endif // BIKE_H
