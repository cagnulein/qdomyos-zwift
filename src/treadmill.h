#ifndef TREADMILL_H
#define TREADMILL_H
#include <QObject>
#include "trainprogram.h"

class treadmill:public QObject
{
    Q_OBJECT

public:
    treadmill();
    virtual unsigned char currentHeart();
    virtual double currentSpeed();
    virtual double currentInclination();
    trainprogram* trainProgram = 0;

public slots:
    virtual void start();
    virtual void stop();
    virtual void changeSpeed(double speed);
    virtual void changeInclination(double inclination);
    virtual void changeSpeedAndInclination(double speed, double inclination);

protected:
    double elapsed = 0;
    double Speed = 0;
    double Inclination = 0;
    uint8_t Heart = 0;
    double requestSpeed = -1;
    double requestInclination = -1;
    int8_t requestStart = -1;
    int8_t requestStop = -1;    
};

#endif // TREADMILL_H
