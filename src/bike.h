#ifndef BIKE_H
#define BIKE_H

#include <QObject>
#include "bluetoothdevice.h"

class bike:public bluetoothdevice
{
    Q_OBJECT

public:
    bike();
    virtual int8_t currentResistance();
    virtual uint8_t avgResistance();
    virtual uint8_t currentCadence();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    virtual uint16_t watts();
    bluetoothdevice::BLUETOOTH_TYPE deviceType();

    int8_t maxResistance = 0;

public slots:
    virtual void changeResistance(int8_t res);

signals:
    void bikeStarted();

protected:
    uint8_t Cadence = 0;
    int8_t Resistance = 0;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double CrankRevs = 0;

    uint64_t totResistance = 0;
    uint32_t countResistance = 0;
};

#endif // BIKE_H
