#include "bluetoothdevice.h"
#include <QTime>
#include <QSettings>

bluetoothdevice::bluetoothdevice()
{

}

bluetoothdevice::BLUETOOTH_TYPE bluetoothdevice::deviceType() { return bluetoothdevice::UNKNOWN; }
void bluetoothdevice::start(){ requestStart = 1; }
void bluetoothdevice::stop(){ requestStop = 1; }
metric bluetoothdevice::currentHeart(){ return Heart; }
metric bluetoothdevice::currentSpeed(){ return Speed; }
QTime bluetoothdevice::movingTime() {int hours = (int)(moving.value()/3600.0); return QTime(hours, (int)(moving.value()-((double)hours * 3600.0)) / 60.0, ((uint32_t) moving.value()) % 60,0); }
QTime bluetoothdevice::elapsedTime() {int hours = (int)(elapsed.value()/3600.0); return QTime(hours, (int)(elapsed.value()-((double)hours * 3600.0)) / 60.0, ((uint32_t) elapsed.value()) % 60,0); }
QTime bluetoothdevice::lapElapsedTime() {int hours = (int)(elapsed.lapValue()/3600.0); return QTime(hours, (int)(elapsed.lapValue()-((double)hours * 3600.0)) / 60.0, ((uint32_t) elapsed.lapValue()) % 60,0); }

void bluetoothdevice::offsetElapsedTime(int offset)
{
    elapsed += offset;
}

QTime bluetoothdevice::currentPace()
{
    QSettings settings;
    bool miles = settings.value("miles_unit", false).toBool();
    double unit_conversion = 1.0;
    if(miles)
        unit_conversion = 0.621371;
    if(Speed.value() == 0)
    {
        return QTime(0,0,0,0);
    }
    else
    {
        double speed = Speed.value() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)), (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0  );
    }
}

QTime bluetoothdevice::averagePace()
{
    QSettings settings;
    bool miles = settings.value("miles_unit", false).toBool();
    double unit_conversion = 1.0;
    if(miles)
        unit_conversion = 0.621371;
    if(Speed.average() == 0)
    {
        return QTime(0,0,0,0);
    }
    else
    {
        double speed = Speed.average() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)), (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0  );
    }
}

QTime bluetoothdevice::maxPace()
{
    QSettings settings;
    bool miles = settings.value("miles_unit", false).toBool();
    double unit_conversion = 1.0;
    if(miles)
        unit_conversion = 0.621371;
    if(Speed.max() == 0)
    {
        return QTime(0,0,0,0);
    }
    else
    {
        double speed = Speed.max() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)), (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0  );
    }
}

double bluetoothdevice::odometer(){ return Distance.value(); }
double bluetoothdevice::calories(){ return KCal.value(); }
metric bluetoothdevice::jouls() { return m_jouls; }
uint8_t bluetoothdevice::fanSpeed() { return FanSpeed; };
void* bluetoothdevice::VirtualDevice() { return nullptr; }
bool bluetoothdevice::changeFanSpeed(uint8_t speed) { Q_UNUSED(speed); return false; }
bool bluetoothdevice::connected() { return false; }
double bluetoothdevice::elevationGain(){ return elevationAcc; }
void bluetoothdevice::heartRate(uint8_t heart) { Heart.setValue(heart); }
void bluetoothdevice::disconnectBluetooth() {if(m_control) m_control->disconnectFromDevice();}
metric bluetoothdevice::wattsMetric() {return m_watt;}
void bluetoothdevice::setDifficult(double d) {m_difficult = d;}
double bluetoothdevice::difficult() {return m_difficult;}
void bluetoothdevice::cadenceSensor(uint8_t cadence) { Q_UNUSED(cadence) }

void bluetoothdevice::update_metrics(const bool watt_calc, const double watts)
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
           m_jouls += (m_watt.value() * deltaTime);
           WeightLoss = metric::calculateWeightLoss(KCal.value());
           if(watt_calc)
               m_watt = watts;
       }
       else if(m_watt.value() > 0)
       {
           m_watt = 0;
       }
    }
    else if(m_watt.value() > 0)
    {
        m_watt = 0;
    }

    _lastTimeUpdate = current;
    _firstUpdate = false;
}

void bluetoothdevice::clearStats()
{
    elapsed.clear(true);
    moving.clear(true);
    Speed.clear(false);
    KCal.clear(true);
    Distance.clear(true);
    Heart.clear(false);
    m_jouls.clear(true);
    elevationAcc = 0;
    m_watt.clear(false);
    WeightLoss.clear(false);
}

void bluetoothdevice::setPaused(bool p)
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
}

void bluetoothdevice::setLap()
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
}
