#include "elliptical.h"


elliptical::elliptical()
{

}

void elliptical::changeResistance(int8_t resistance) { requestResistance = resistance; RequestedResistance = resistance;}
double elliptical::currentCrankRevolutions() { return CrankRevs;}
uint16_t elliptical::lastCrankEventTime() { return LastCrankEventTime;}
int8_t elliptical::currentResistance() { return Resistance;}
uint8_t elliptical::currentCadence() { return Cadence;}
uint8_t elliptical::fanSpeed() { return FanSpeed; }
bool elliptical::connected() { return false; }
uint16_t elliptical::watts() { return 0; }

bluetoothdevice::BLUETOOTH_TYPE elliptical::deviceType() { return bluetoothdevice::ELLIPTICAL; }
