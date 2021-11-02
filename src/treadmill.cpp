
#include "treadmill.h"
#include <QSettings>

treadmill::treadmill() {}

void treadmill::changeSpeed(double speed) { requestSpeed = speed; }
void treadmill::changeInclination(double inclination) {
    if (autoResistanceEnable) {
        requestInclination = inclination;
    }
}
bool treadmill::changeFanSpeed(uint8_t speed) {
    requestFanSpeed = speed;
    return true;
}
void treadmill::changeSpeedAndInclination(double speed, double inclination) {
    requestSpeed = speed;
    requestInclination = inclination;
}
metric treadmill::currentInclination() { return Inclination; }
uint8_t treadmill::fanSpeed() { return FanSpeed; };
bool treadmill::connected() { return false; }
bluetoothdevice::BLUETOOTH_TYPE treadmill::deviceType() { return bluetoothdevice::TREADMILL; }

double treadmill::minStepInclination() { return 0.5; }

void treadmill::update_metrics(bool watt_calc, const double watts) {

    QDateTime current = QDateTime::currentDateTime();
    double deltaTime = (((double)_lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
    QSettings settings;
    if (!_firstUpdate && !paused) {
        if (currentSpeed().value() > 0.0 || settings.value(QStringLiteral("continuous_moving"), true).toBool()) {
            elapsed += deltaTime;
        }
        if (currentSpeed().value() > 0.0) {

            moving += deltaTime;
            if (watt_calc) {
                m_watt = watts;
            }
            m_jouls += (m_watt.value() * deltaTime);
            WeightLoss = metric::calculateWeightLoss(KCal.value());
            WattKg = m_watt.value() / settings.value(QStringLiteral("weight"), 75.0).toFloat();
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

uint16_t treadmill::watts(double weight) {

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

void treadmill::clearStats() {

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

void treadmill::setPaused(bool p) {

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

void treadmill::setLap() {

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

void treadmill::setLastSpeed(double speed) { lastSpeed = speed; }

void treadmill::setLastInclination(double inclination) { lastInclination = inclination; }

bool treadmill::autoPauseWhenSpeedIsZero() { return false; }
bool treadmill::autoStartWhenSpeedIsGreaterThenZero() { return false; }

double treadmill::requestedSpeed() { return requestSpeed; }
double treadmill::requestedInclination() { return requestInclination; }
double treadmill::currentTargetSpeed() { return targetSpeed; }
