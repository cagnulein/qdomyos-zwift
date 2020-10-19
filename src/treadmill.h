#ifndef TREADMILL_H
#define TREADMILL_H
#include <QObject>
#include "bluetoothdevice.h"

class treadmill:public bluetoothdevice
{
    Q_OBJECT

public:
    treadmill();    
    virtual double currentInclination();
    virtual double elevationGain();
    virtual uint8_t fanSpeed();
    virtual bool connected();
    uint16_t watts(double weight=75.0);    
    bluetoothdevice::BLUETOOTH_TYPE deviceType();

public slots:
    virtual void changeSpeed(double speed);
    virtual void changeInclination(double inclination);
    virtual void changeSpeedAndInclination(double speed, double inclination);

signals:
    void tapeStarted();

protected:
    double elevationAcc = 0;
    double Inclination = 0;
    double requestSpeed = -1;
    double requestInclination = -1;
};

#endif // TREADMILL_H
