#include <QDebug>
#include "rower.h"

rower::rower()
{

}

void rower::changeResistance(int8_t resistance) { if(autoResistanceEnable) {requestResistance = resistance * m_difficult; emit resistanceChanged(requestResistance);} RequestedResistance = resistance * m_difficult; }
void rower::changeRequestedPelotonResistance(int8_t resistance) { RequestedPelotonResistance = resistance; }
void rower::changeCadence(int16_t cadence) { RequestedCadence = cadence; }
void rower::changePower(int32_t power) { RequestedPower = power; }
double rower::currentCrankRevolutions() { return CrankRevs;}
uint16_t rower::lastCrankEventTime() { return LastCrankEventTime;}
metric rower::lastRequestedResistance() { return RequestedResistance; }
metric rower::lastRequestedPelotonResistance() { return RequestedPelotonResistance; }
metric rower::lastRequestedCadence() { return RequestedCadence; }
metric rower::lastRequestedPower() { return RequestedPower; }
metric rower::currentResistance() { return Resistance;}
metric rower::currentCadence() { return Cadence;}
uint8_t rower::fanSpeed() { return FanSpeed; }
bool rower::connected() { return false; }
uint16_t rower::watts() { return 0; }
metric rower::pelotonResistance() { return m_pelotonResistance; }
int rower::pelotonToBikeResistance(int pelotonResistance) {return pelotonResistance;}
uint8_t rower::resistanceFromPowerRequest(uint16_t power) {return power / 10;} // in order to have something
void rower::cadenceSensor(uint8_t cadence) { Cadence.setValue(cadence); }

bluetoothdevice::BLUETOOTH_TYPE rower::deviceType() { return bluetoothdevice::ROWING; }

void rower::clearStats()
{
    moving.clear(true);
    elapsed.clear(true);
    Speed.clear(false);
    KCal.clear(true);
    Distance.clear(true);
    Heart.clear(false);
    m_jouls.clear(true);
    elevationAcc = 0;
    m_watt.clear(false);
    WeightLoss.clear(false);

    RequestedPelotonResistance.clear(false);
    RequestedResistance.clear(false);
    RequestedCadence.clear(false);
    RequestedPower.clear(false);
    m_pelotonResistance.clear(false);
    Cadence.clear(false);
    Resistance.clear(false);
}

void rower::setPaused(bool p)
{
    paused = p;
    moving.setPaused(p);
    elapsed.setPaused(p);
    Speed.setPaused(p);
    KCal.setPaused(p);
    Distance.setPaused(p);
    Heart.setPaused(p);
    m_jouls.setPaused(p);
    m_watt.setPaused(p);
    WeightLoss.setPaused(p);
    m_pelotonResistance.setPaused(p);
    Cadence.setPaused(p);
    Resistance.setPaused(p);
    RequestedPelotonResistance.setPaused(p);
    RequestedResistance.setPaused(p);
    RequestedCadence.setPaused(p);
    RequestedPower.setPaused(p);
}

void rower::setLap()
{
    moving.setLap(true);
    elapsed.setLap(true);
    Speed.setLap(false);
    KCal.setLap(true);
    Distance.setLap(true);
    Heart.setLap(false);
    m_jouls.setLap(true);
    m_watt.setLap(false);
    WeightLoss.setLap(false);

    RequestedPelotonResistance.setLap(false);
    RequestedResistance.setLap(false);
    RequestedCadence.setLap(false);
    RequestedPower.setLap(false);
    m_pelotonResistance.setLap(false);
    Cadence.setLap(false);
    Resistance.setLap(false);
}
