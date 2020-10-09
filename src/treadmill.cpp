#include "treadmill.h"

treadmill::treadmill()
{

}

void treadmill::start(){ requestStart = 1; }
void treadmill::stop(){ requestStop = 1; }
void treadmill::changeSpeed(double speed){ requestSpeed = speed;}
void treadmill::changeInclination(double inclination){ requestInclination = inclination; }
unsigned char treadmill::currentHeart(){ return Heart; }
double treadmill::currentSpeed(){ return Speed; }
double treadmill::currentInclination(){ return Inclination; }
