#include "bluetoothdevice.h"
#include <QTime>

bluetoothdevice::bluetoothdevice()
{

}

bluetoothdevice::BLUETOOTH_TYPE bluetoothdevice::deviceType() { return bluetoothdevice::UNKNOWN; }
void bluetoothdevice::start(){ requestStart = 1; }
void bluetoothdevice::stop(){ requestStop = 1; }
unsigned char bluetoothdevice::currentHeart(){ return Heart; }
double bluetoothdevice::currentSpeed(){ return Speed; }
QTime bluetoothdevice::currentPace(){ return QTime(0, (int)(1.0 / (Speed / 60.0)), (((double)(1.0 / (Speed / 60.0)) - ((double)((int)(1.0 / (Speed / 60.0))))) * 60.0), 0  ); }
double bluetoothdevice::odometer(){ return Distance; }
double bluetoothdevice::calories(){ return KCal; }
uint8_t bluetoothdevice::fanSpeed() { return FanSpeed; };
void* bluetoothdevice::VirtualDevice() { return nullptr; }
bool bluetoothdevice::changeFanSpeed(uint8_t speed) { Q_UNUSED(speed); return false; }
bool bluetoothdevice::connected() { return false; }
