#include "treadmill.h"

treadmill::treadmill()
{

}

void treadmill::changeSpeed(double speed){ requestSpeed = speed;}
void treadmill::changeInclination(double inclination){ requestInclination = inclination; }
void treadmill::changeSpeedAndInclination(double speed, double inclination){ requestSpeed = speed; requestInclination = inclination;}
double treadmill::currentInclination(){ return Inclination; }
double treadmill::odometer(){ return Distance; }
double treadmill::elevationGain(){ return elevationAcc; }
double treadmill::calories(){ return KCal; }
uint8_t treadmill::fanSpeed() { return FanSpeed; };
bool treadmill::connected() { return false; }
bluetoothdevice::BLUETOOTH_TYPE treadmill::deviceType() { return bluetoothdevice::TREADMILL; }

uint16_t treadmill::watts(double weight)
{
    // calc Watts ref. https://alancouzens.com/blog/Run_Power.html

    uint16_t watts=0;
    if(currentSpeed() > 0)
    {
       double pace=60/currentSpeed();
       double VO2R=210.0/pace;
       double VO2A=(VO2R*weight)/1000.0;
       double hwatts=75*VO2A;
       double vwatts=((9.8*weight) * (currentInclination()/100));
       watts=hwatts+vwatts;
    }
    return watts;
}
