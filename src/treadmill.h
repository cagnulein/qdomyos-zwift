#ifndef TREADMILL_H
#define TREADMILL_H
#include <QObject>

class treadmill:public QObject
{
    Q_OBJECT

public:
    treadmill();
    virtual void start();
    virtual void stop();
    virtual void changeSpeed(double speed);
    virtual void changeInclination(double inclination);
    virtual unsigned char currentHeart();
    virtual double currentSpeed();
    virtual double currentInclination();

protected:
    double Speed = 0;
    double Inclination = 0;
    uint8_t Heart = 0;
    double requestSpeed = -1;
    double requestInclination = -1;
    int8_t requestStart = -1;
    int8_t requestStop = -1;
};

#endif // TREADMILL_H
