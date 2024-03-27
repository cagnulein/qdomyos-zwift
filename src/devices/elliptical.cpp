
#include "devices/elliptical.h"
#include <QSettings>

elliptical::elliptical() {}

void elliptical::update_metrics(bool watt_calc, const double watts) {

    QDateTime current = QDateTime::currentDateTime();
    double deltaTime = (((double)_lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
    QSettings settings;
    if (!_firstUpdate && !paused) {
        if (currentSpeed().value() > 0.0 || settings.value(QZSettings::continuous_moving, true).toBool()) {
            elapsed += deltaTime;
        }
        if (currentSpeed().value() > 0.0) {
            moving += deltaTime;
            if (watt_calc) {
                m_watt = watts;
            }
            m_jouls += (m_watt.value() * deltaTime);
            WeightLoss = metric::calculateWeightLoss(KCal.value());
            WattKg = m_watt.value() / settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
        } else if (m_watt.value() > 0) {
            m_watt = 0;
            WattKg = 0;
        }
    } else if (m_watt.value() > 0) {
        m_watt = 0;
        WattKg = 0;
    }

    METS = calculateMETS();
    elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;

    _lastTimeUpdate = current;
    _firstUpdate = false;
}

uint16_t elliptical::watts() {

    QSettings settings;
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    // calc Watts ref. https://alancouzens.com/blog/Run_Power.html

    uint16_t watts = 0;
    if (currentSpeed().value() > 0) {

        double pace = 60 / currentSpeed().value();
        double VO2R = 210.0 / pace;
        double VO2A = (VO2R * weight) / 1000.0;
        double hwatts = 75 * VO2A;
        double vwatts = ((9.8 * weight) * (currentInclination().value() / 100.0));
        watts = hwatts + vwatts;
    }
    m_watt.setValue(watts);
    return m_watt.value();
}

double elliptical::speedFromWatts() {

    QSettings settings;
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    // calc Watts ref. https://alancouzens.com/blog/Run_Power.html

    double speed = 0;
    if (wattsMetric().value() > 0) {
        double vwatts = ((9.8 * weight) * (currentInclination().value() / 100.0));
        speed = 210.0 / ((wattsMetric().value() - vwatts) / 75.0 / weight * 1000.0);
        speed = 60.0 / speed;
    }
    return speed;
}

void elliptical::changeResistance(resistance_t resistance) {
    qDebug() << "changeResistance" << resistance;
    lastRawRequestedResistanceValue = resistance;
    requestResistance = resistance + gears();
    RequestedResistance = resistance + gears();
}
double elliptical::gears() { return m_gears; }
void elliptical::setGears(double gears) {
    QSettings settings;
    qDebug() << "setGears" << gears;
    m_gears = gears;
    settings.setValue(QZSettings::gears_current_value, m_gears);
    if (lastRawRequestedResistanceValue != -1) {
        changeResistance(lastRawRequestedResistanceValue);
    }
}
void elliptical::changeInclination(double grade, double inclination) {
    qDebug() << "changeInclination" << grade << inclination;
    if (autoResistanceEnable) {
        requestInclination = inclination;
    }
}
double elliptical::currentCrankRevolutions() { return CrankRevs; }
uint16_t elliptical::lastCrankEventTime() { return LastCrankEventTime; }
metric elliptical::currentResistance() { return Resistance; }
metric elliptical::currentInclination() { return Inclination; }
uint8_t elliptical::fanSpeed() { return FanSpeed; }
bool elliptical::connected() { return false; }

bluetoothdevice::BLUETOOTH_TYPE elliptical::deviceType() { return bluetoothdevice::ELLIPTICAL; }

void elliptical::clearStats() {
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
    WattKg.clear(false);
    Inclination.clear(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].clear(false);
    }    
}

void elliptical::setPaused(bool p) {
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
    Inclination.setPaused(p);
    WeightLoss.setPaused(p);
    WattKg.setPaused(p);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setPaused(p);
    }    
}

void elliptical::setLap() {
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
    WattKg.setLap(false);

    Inclination.setLap(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setLap(false);
    }
}

int elliptical::pelotonToEllipticalResistance(int pelotonResistance) { return pelotonResistance; }
void elliptical::changeCadence(int16_t cadence) { RequestedCadence = cadence; }
void elliptical::changeRequestedPelotonResistance(int8_t resistance) { RequestedPelotonResistance = resistance; }
double elliptical::requestedSpeed() { return requestSpeed; }
void elliptical::changeSpeed(double speed) {
    RequestedSpeed = speed;
    if (autoResistanceEnable)
        requestSpeed = speed;
}
metric elliptical::lastRequestedCadence() { return RequestedCadence; }
metric elliptical::pelotonResistance() { return m_pelotonResistance; }
metric elliptical::lastRequestedPelotonResistance() { return RequestedPelotonResistance; }
metric elliptical::lastRequestedResistance() { return RequestedResistance; }
bool elliptical::inclinationAvailableByHardware() { return true; }
