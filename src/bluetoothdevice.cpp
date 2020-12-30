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
QTime bluetoothdevice::elapsedTime() {int hours = (int)(elapsed/3600.0); return QTime(hours, (int)(elapsed-((double)hours * 3600.0)) / 60.0, ((uint32_t) elapsed) % 60,0); }
QTime bluetoothdevice::currentPace(){ if(Speed == 0) return QTime(0,0,0,0); else return QTime(0, (int)(1.0 / (Speed / 60.0)), (((double)(1.0 / (Speed / 60.0)) - ((double)((int)(1.0 / (Speed / 60.0))))) * 60.0), 0  ); }
double bluetoothdevice::odometer(){ return Distance; }
double bluetoothdevice::calories(){ return KCal; }
double bluetoothdevice::jouls() { return m_jouls; }
uint8_t bluetoothdevice::fanSpeed() { return FanSpeed; };
void* bluetoothdevice::VirtualDevice() { return nullptr; }
bool bluetoothdevice::changeFanSpeed(uint8_t speed) { Q_UNUSED(speed); return false; }
bool bluetoothdevice::connected() { return false; }
uint8_t bluetoothdevice::pelotonResistance() { return m_pelotonResistance; }
double bluetoothdevice::elevationGain(){ return elevationAcc; }
double bluetoothdevice::avgWatt(){ if(countPower) return (double)totPower/(double)countPower; else return 0;  }
void bluetoothdevice::heartRate(uint8_t heart) { Heart = heart; }
