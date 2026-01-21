
#include "rower.h"
#include "qdebugfixup.h"
#include <QSettings>

rower::rower() {}

void rower::changeSpeed(double speed) {
    qDebug() << "changeSpeed" << speed;
    RequestedSpeed = speed;
    if (autoResistanceEnable)
        requestSpeed = speed;
}
void rower::changeResistance(resistance_t resistance) {
    lastRawRequestedResistanceValue = resistance;
    if (autoResistanceEnable) {
        requestResistance = (resistance * m_difficult) + gears();;
        emit resistanceChanged(requestResistance);
    }
    RequestedResistance = (resistance * m_difficult) + gears();;
}

double rower::gears() { return m_gears; }
void rower::setGears(double gears) {
    QSettings settings;
    qDebug() << "setGears" << gears;
    m_gears = gears;
    if (settings.value(QZSettings::gears_restore_value, QZSettings::default_gears_restore_value).toBool())
        settings.setValue(QZSettings::gears_current_value, m_gears);
    if (lastRawRequestedResistanceValue != -1) {
        changeResistance(lastRawRequestedResistanceValue);
    }
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
double rower::requestedSpeed() { return requestSpeed; }

double rower::calculateWattsFromPace(double paceSeconds) {
    // Concept2 Rowing Power Formula: Watts = 2.8 / (pace_in_seconds / 500)^3
    // Which simplifies to: Watts = 2.8 * (500 / pace_in_seconds)^3
    if (paceSeconds <= 0)
        return 0;

    double velocity = 500.0 / paceSeconds;  // meters per second for 500m split
    return 2.8 * velocity * velocity * velocity;
}

BLUETOOTH_TYPE rower::deviceType() { return ROWING; }

void rower::clearStats() {

    moving.clear(true);
    elapsed.clear(true);
    Speed.clear(false);
    KCal.clear(true);
    Distance.clear(true);
    Distance1s.clear(true);
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

    speedLast500mValues.clear();

    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].clear(false);
    }    
}

void rower::setPaused(bool p) {

    paused = p;
    moving.setPaused(p);
    elapsed.setPaused(p);
    Speed.setPaused(p);
    KCal.setPaused(p);
    Distance.setPaused(p);
    Distance1s.setPaused(p);
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

    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setPaused(p);
    }    
}

void rower::setLap() {

    moving.setLap(true);
    elapsed.setLap(true);
    Speed.setLap(false);
    KCal.setLap(true);
    Distance.setLap(true);
    Distance1s.setLap(true);
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

    speedLast500mValues.clear();
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setLap(false);
    }    
}

// min/500m
QTime rower::averagePace() {

    QSettings settings;
    // bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    /*if (miles) {
        unit_conversion = 0.621371;
    }*/
    if (Speed.average() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.average() * unit_conversion * 2.0; //*2 in order to change from min/km to min/500m
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

// min/500m
QTime rower::maxPace() {

    QSettings settings;
    // bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    /*if (miles) {
        unit_conversion = 0.621371;
    }*/
    if (Speed.max() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.max() * unit_conversion * 2.0; //*2 in order to change from min/km to min/500m
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

// min/500m
QTime rower::currentPace() { return speedToPace(Speed.value()); }

// min/500m
QTime rower::lastPace500m() {

    QSettings settings;
    // bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    const double unit_conversion = 1.0;
    // rowers are always in meters!
    /*if (miles) {
        unit_conversion = 0.621371;
    }*/

    // last 500m speed calculation
    if (!paused && Speed.value() > 0) {
        double o = odometer();
        speedLast500mValues.append(new rowerSpeedDistance(o, Speed.value()));
        while (o > speedLast500mValues.first()->distance + 0.5) {
            delete speedLast500mValues.first();
            speedLast500mValues.removeFirst();
        }
    }

    if (speedLast500mValues.count() == 0)
        return QTime(0, 0, 0, 0);

    double avg = 0;
    for (int i = 0; i < speedLast500mValues.count(); i++)
        avg += speedLast500mValues.at(i)->speed;
    avg = avg / (double)speedLast500mValues.count();

    double speed = avg * unit_conversion * 2.0; //*2 in order to change from min/km to min/500m
    return QTime(0, (int)(1.0 / (speed / 60.0)),
                 (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
}

// min/500m
QTime rower::lastRequestedPace() { return speedToPace(lastRequestedSpeed().value()); }

// min/500m
QTime rower::speedToPace(double Speed) {
    QSettings settings;
    // bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    const double unit_conversion = 1.0;
    // rowers are always in meters!
    /*if (miles) {
        unit_conversion = 0.621371;
    }*/
    if (Speed == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed * unit_conversion * 2.0; //*2 in order to change from min/km to min/500m
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}
