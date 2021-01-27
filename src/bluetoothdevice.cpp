#include "bluetoothdevice.h"
#include <QTime>

bluetoothdevice::bluetoothdevice()
{

}

bluetoothdevice::BLUETOOTH_TYPE bluetoothdevice::deviceType() { return bluetoothdevice::UNKNOWN; }
void bluetoothdevice::start(){ requestStart = 1; }
void bluetoothdevice::stop(){ requestStop = 1; }
metric bluetoothdevice::currentHeart(){ return Heart; }
metric bluetoothdevice::currentSpeed(){ return Speed; }
QTime bluetoothdevice::elapsedTime() {int hours = (int)(elapsed.value()/3600.0); return QTime(hours, (int)(elapsed.value()-((double)hours * 3600.0)) / 60.0, ((uint32_t) elapsed.value()) % 60,0); }
QTime bluetoothdevice::currentPace(){ if(Speed.value() == 0) return QTime(0,0,0,0); else return QTime(0, (int)(1.0 / (Speed.value() / 60.0)), (((double)(1.0 / (Speed.value() / 60.0)) - ((double)((int)(1.0 / (Speed.value() / 60.0))))) * 60.0), 0  ); }
double bluetoothdevice::odometer(){ return Distance.value(); }
double bluetoothdevice::calories(){ return KCal.value(); }
metric bluetoothdevice::jouls() { return m_jouls; }
uint8_t bluetoothdevice::fanSpeed() { return FanSpeed; };
void* bluetoothdevice::VirtualDevice() { return nullptr; }
bool bluetoothdevice::changeFanSpeed(uint8_t speed) { Q_UNUSED(speed); return false; }
bool bluetoothdevice::connected() { return false; }
double bluetoothdevice::elevationGain(){ return elevationAcc; }
void bluetoothdevice::heartRate(uint8_t heart) { Heart.setValue(heart); }
void bluetoothdevice::disconnect() {if(m_control) m_control->disconnect();}
metric bluetoothdevice::wattsMetric() {return m_watt;}

void bluetoothdevice::clearStats()
{
    elapsed.clear(true);
    Speed.clear(false);
    KCal.clear(true);
    Distance.clear(true);
    Heart.clear(false);
    m_jouls.clear(true);
    elevationAcc = 0;
    m_watt.clear(false);
}

void bluetoothdevice::setPaused(bool p)
{
    paused = p;
    elapsed.setPaused(p);
    Speed.setPaused(p);
    KCal.setPaused(p);
    Distance.setPaused(p);
    Heart.setPaused(p);
    m_jouls.setPaused(p);
    m_watt.setPaused(p);
}
