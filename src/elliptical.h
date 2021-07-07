#ifndef ELLIPTICAL_H
#define ELLIPTICAL_H
#include <QObject>
#include "bluetoothdevice.h"

class elliptical:public bluetoothdevice
{
    Q_OBJECT

public:
    elliptical();
    void update_metrics(const bool watt_calc, const double watts);
    metric lastRequestedResistance();
    virtual metric currentInclination();
    virtual metric currentResistance();
    virtual metric currentCadence();
    virtual uint8_t fanSpeed();
    virtual double currentCrankRevolutions();
    virtual uint16_t lastCrankEventTime();
    virtual bool connected();    
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    void clearStats();
    void setPaused(bool p);
    void setLap();
    uint16_t watts();

public slots:
    virtual void changeResistance(int8_t res);
    virtual void changeInclination(double inclination);

signals:
    void bikeStarted();

protected:    

    metric Inclination;
    double requestInclination = -1;
    metric RequestedResistance;
    metric Cadence;
    metric Resistance;
    uint16_t LastCrankEventTime = 0;
    int8_t requestResistance = -1;
    double CrankRevs = 0;
};

#endif // ELLIPTICAL_H
