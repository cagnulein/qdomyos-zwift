#ifndef BIKE_H
#define BIKE_H

#include <QObject>
#include "bluetoothdevice.h"

class bike:public bluetoothdevice
{
    Q_OBJECT

public:
    bike();
    virtual metric currentResistance();
    virtual metric currentCadence();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    virtual uint16_t watts();
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    metric pelotonResistance();

public slots:
    virtual void changeResistance(int8_t res);

signals:
    void bikeStarted();

protected:
    metric Cadence;
    metric Resistance;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double CrankRevs = 0;

    metric m_pelotonResistance;
};

#endif // BIKE_H
