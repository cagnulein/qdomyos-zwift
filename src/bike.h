#ifndef BIKE_H
#define BIKE_H

#include <QObject>
#include "bluetoothdevice.h"

class bike:public bluetoothdevice
{
    Q_OBJECT

public:
    bike();
    virtual double currentResistance();
    virtual uint8_t fanSpeed();
    virtual bool connected();
    uint16_t watts(double weight=75.0);
    bluetoothdevice::BLUETOOTH_TYPE deviceType();

public slots:
    virtual void changeResistance(double res);

signals:
    void bikeStarted();

protected:
    double Resistance = 0;
    double requestResistance = -1;
};

#endif // BIKE_H
