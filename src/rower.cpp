
#include "rower.h"
#include "qdebugfixup.h"
#include <QSettings>

rower::rower() {}

void rower::changeResistance(resistance_t resistance) {
    if (autoResistanceEnable) {
        requestResistance = resistance * m_difficult;
        emit resistanceChanged(requestResistance);
    }
    RequestedResistance = resistance * m_difficult;
}

void rower::changeRequestedPelotonResistance(int8_t resistance) { RequestedPelotonResistance = resistance; }
void rower::changeCadence(int16_t cadence) { RequestedCadence = cadence; }
void rower::changePower(int32_t power) {
    RequestedPower = power;
    qDebug() << "rower::changePower" << power;
}
double rower::currentCrankRevolutions() { return CrankRevs; }
uint16_t rower::lastCrankEventTime() { return LastCrankEventTime; }
metric rower::lastRequestedResistance() { return RequestedResistance; }
metric rower::lastRequestedPelotonResistance() { return RequestedPelotonResistance; }
metric rower::lastRequestedCadence() { return RequestedCadence; }
metric rower::lastRequestedPower() { return RequestedPower; }
metric rower::currentResistance() { return Resistance; }
metric rower::currentStrokesCount() { return StrokesCount; }
metric rower::currentStrokesLength() { return StrokesLength; }
uint8_t rower::fanSpeed() { return FanSpeed; }
bool rower::connected() { return false; }
uint16_t rower::watts() { return 0; }
metric rower::pelotonResistance() { return m_pelotonResistance; }
resistance_t rower::pelotonToBikeResistance(int pelotonResistance) { return pelotonResistance; }
resistance_t rower::resistanceFromPowerRequest(uint16_t power) { return power / 10; } // in order to have something
void rower::cadenceSensor(uint8_t cadence) { Cadence.setValue(cadence); }
void rower::powerSensor(uint16_t power) { m_watt.setValue(power, false); }

bluetoothdevice::BLUETOOTH_TYPE rower::deviceType() { return bluetoothdevice::ROWING; }

void rower::clearStats() {

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
    StrokesCount.clear(false);
    StrokesLength.clear(false);

    RequestedPelotonResistance.clear(false);
    RequestedResistance.clear(false);
    RequestedCadence.clear(false);
    RequestedPower.clear(false);
    m_pelotonResistance.clear(false);
    Cadence.clear(false);
    Resistance.clear(false);
    WattKg.clear(false);
}

void rower::setPaused(bool p) {

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
    StrokesCount.setPaused(p);
    StrokesLength.setPaused(p);
    WattKg.setPaused(p);
}

void rower::setLap() {

    moving.setLap(true);
    elapsed.setLap(true);
    Speed.setLap(false);
    KCal.setLap(true);
    Distance.setLap(true);
    Heart.setLap(false);
    m_jouls.setLap(true);
    m_watt.setLap(false);
    WeightLoss.setLap(false);
    StrokesCount.setLap(false);
    StrokesLength.setLap(false);

    RequestedPelotonResistance.setLap(false);
    RequestedResistance.setLap(false);
    RequestedCadence.setLap(false);
    RequestedPower.setLap(false);
    m_pelotonResistance.setLap(false);
    Cadence.setLap(false);
    Resistance.setLap(false);
    WattKg.setLap(false);
}

// min/500m
QTime rower::currentPace() {
    QSettings settings;
    // bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    const double unit_conversion = 1.0;
    // rowers are always in meters!
    /*if (miles) {
        unit_conversion = 0.621371;
    }*/
    if (Speed.value() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.value() * unit_conversion * 2.0; //*2 in order to change from min/km to min/500m
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}
