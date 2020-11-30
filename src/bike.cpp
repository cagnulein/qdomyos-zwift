#include <QDebug>
#include "bike.h"

bike::bike()
{

}

void bike::changeResistance(int8_t resistance) { requestResistance = resistance;}
double bike::currentCrankRevolutions() { return CrankRevs;}
uint16_t bike::lastCrankEventTime() { return LastCrankEventTime;}
int8_t bike::currentResistance() { return Resistance;}
uint8_t bike::currentCadence() { return Cadence;}
uint8_t bike::fanSpeed() { return FanSpeed; }
bool bike::connected() { return false; }
uint16_t bike::watts() { return 0; }

bluetoothdevice::BLUETOOTH_TYPE bike::deviceType() { return bluetoothdevice::BIKE; }
