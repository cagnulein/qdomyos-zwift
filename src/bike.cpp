#include <QDebug>
#include "bike.h"

bike::bike()
{

}

void bike::changeResistance(int8_t resistance) { if(autoResistanceEnable) {requestResistance = resistance * m_difficult;} RequestedResistance = resistance * m_difficult; }
void bike::changeCadence(int16_t cadence) { RequestedCadence = cadence; }
void bike::changePower(int32_t power) { RequestedPower = power; }
double bike::currentCrankRevolutions() { return CrankRevs;}
uint16_t bike::lastCrankEventTime() { return LastCrankEventTime;}
metric bike::lastRequestedResistance() { return RequestedResistance; }
metric bike::lastRequestedCadence() { return RequestedCadence; }
metric bike::lastRequestedPower() { return RequestedPower; }
metric bike::currentResistance() { return Resistance;}
metric bike::currentCadence() { return Cadence;}
uint8_t bike::fanSpeed() { return FanSpeed; }
bool bike::connected() { return false; }
uint16_t bike::watts() { return 0; }
metric bike::pelotonResistance() { return m_pelotonResistance; }
int bike::pelotonToBikeResistance(int pelotonResistance) {return pelotonResistance;}
uint8_t bike::resistanceFromPowerRequest(uint16_t power) {return power / 10;} // in order to have something

bluetoothdevice::BLUETOOTH_TYPE bike::deviceType() { return bluetoothdevice::BIKE; }

void bike::clearStats()
{
    elapsed.clear(true);
    Speed.clear(false);
    KCal.clear(true);
    Distance.clear(true);
    Heart.clear(false);
    m_jouls.clear(true);
    elevationAcc = 0;
    m_watt.clear(false);

    RequestedResistance.clear(false);
    RequestedCadence.clear(false);
    RequestedPower.clear(false);
    m_pelotonResistance.clear(false);
    Cadence.clear(false);
    Resistance.clear(false);
}

void bike::setPaused(bool p)
{
    paused = p;
    elapsed.setPaused(p);
    Speed.setPaused(p);
    KCal.setPaused(p);
    Distance.setPaused(p);
    Heart.setPaused(p);
    m_jouls.setPaused(p);
    m_watt.setPaused(p);
    m_pelotonResistance.setPaused(p);
    Cadence.setPaused(p);
    Resistance.setPaused(p);
    RequestedResistance.setPaused(p);
    RequestedCadence.setPaused(p);
    RequestedPower.setPaused(p);
}

void bike::setLap()
{
    elapsed.setLap(true);
    Speed.setLap(false);
    KCal.setLap(true);
    Distance.setLap(true);
    Heart.setLap(false);
    m_jouls.setLap(true);
    m_watt.setLap(false);

    RequestedResistance.setLap(false);
    RequestedCadence.setLap(false);
    RequestedPower.setLap(false);
    m_pelotonResistance.setLap(false);
    Cadence.setLap(false);
    Resistance.setLap(false);
}
