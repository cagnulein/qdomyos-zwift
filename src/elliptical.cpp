#include <QSettings>
#include "elliptical.h"


elliptical::elliptical()
{

}

void elliptical::update_metrics(const bool watt_calc, const double watts)
{
    QDateTime current = QDateTime::currentDateTime();
    double deltaTime = (((double)_lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
    QSettings settings;
    if(!_firstUpdate && !paused)
    {
       if(currentSpeed().value() > 0.0 || settings.value("continuous_moving", true).toBool())
       {
           elapsed += deltaTime;
       }
       if(currentSpeed().value() > 0.0)
       {
           moving += deltaTime;
           if(watt_calc)
               m_watt = watts;
           m_jouls += (m_watt.value() * deltaTime);
           WeightLoss = metric::calculateWeightLoss(KCal.value());
           WattKg = m_watt.value() / settings.value("weight", 75.0).toFloat();
       }
       else if(m_watt.value() > 0)
       {
           m_watt = 0;
           WattKg = 0;
       }
    }
    else if(m_watt.value() > 0)
    {
        m_watt = 0;
        WattKg = 0;
    }

    elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;

    _lastTimeUpdate = current;
    _firstUpdate = false;
}

uint16_t elliptical::watts() {return 0;}
void elliptical::changeResistance(int8_t resistance) { requestResistance = resistance; RequestedResistance = resistance;}
void elliptical::changeInclination(double inclination){ requestInclination = inclination; }
double elliptical::currentCrankRevolutions() { return CrankRevs;}
uint16_t elliptical::lastCrankEventTime() { return LastCrankEventTime;}
metric elliptical::currentResistance() { return Resistance;}
metric elliptical::currentInclination(){ return Inclination; }
metric elliptical::currentCadence() { return Cadence;}
uint8_t elliptical::fanSpeed() { return FanSpeed; }
bool elliptical::connected() { return false; }

bluetoothdevice::BLUETOOTH_TYPE elliptical::deviceType() { return bluetoothdevice::ELLIPTICAL; }

void elliptical::clearStats()
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
    WattKg.clear(false);

    Inclination.clear(false);
}

void elliptical::setPaused(bool p)
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
    Inclination.setPaused(p);
    WeightLoss.setPaused(p);
    WattKg.setPaused(p);
}

void elliptical::setLap()
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
    WattKg.setLap(false);

    Inclination.setLap(false);
}
