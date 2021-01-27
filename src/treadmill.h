#ifndef TREADMILL_H
#define TREADMILL_H
#include <QObject>
#include "bluetoothdevice.h"

class treadmill:public bluetoothdevice
{
    Q_OBJECT

public:
    treadmill();    
    virtual metric currentInclination();
    virtual uint8_t fanSpeed();
    virtual bool connected();
    uint16_t watts(double weight);
    bluetoothdevice::BLUETOOTH_TYPE deviceType();
    void clearStats();
    void setPaused(bool p);

public slots:
    virtual void changeSpeed(double speed);
    virtual void changeInclination(double inclination);
    virtual void changeSpeedAndInclination(double speed, double inclination);

signals:
    void tapeStarted();

protected:    
    metric Inclination;
    double requestSpeed = -1;
    double requestInclination = -1;
};

#endif // TREADMILL_H
