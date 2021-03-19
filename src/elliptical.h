#ifndef ELLIPTICAL_H
#define ELLIPTICAL_H
#include <QObject>
#include "bluetoothdevice.h"

class elliptical:public bluetoothdevice
{
    Q_OBJECT

public:
    elliptical();
    metric lastRequestedResistance();
    virtual metric currentInclination();
    virtual int8_t currentResistance();
    virtual uint8_t currentCadence();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();
    virtual uint16_t watts();
    bluetoothdevice::BLUETOOTH_TYPE deviceType();

public slots:
    virtual void changeResistance(int8_t res);
    virtual void changeInclination(double inclination);

signals:
    void bikeStarted();

protected:
    metric Inclination;
    double requestInclination = -1;
    metric RequestedResistance;
    uint8_t Cadence = 0;
    int8_t Resistance = 0;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double CrankRevs = 0;
};

#endif // ELLIPTICAL_H
