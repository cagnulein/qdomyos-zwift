#include "treadmill.h"
#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif
#include <QSettings>

treadmill::treadmill() {}

void treadmill::changeSpeed(double speed) {
    QSettings settings;
    m_lastRawSpeedRequested = speed;
    speed /= settings.value(QZSettings::speed_gain, QZSettings::default_speed_gain).toDouble();
    speed -= settings.value(QZSettings::speed_offset, QZSettings::default_speed_offset).toDouble();    
    qDebug() << "changeSpeed" << speed << autoResistanceEnable << m_difficult << m_difficult_offset << m_lastRawSpeedRequested;
    RequestedSpeed = (speed * m_difficult) + m_difficult_offset;
    if (autoResistanceEnable)
        requestSpeed = (speed * m_difficult) + m_difficult_offset;
}
void treadmill::changeInclination(double grade, double inclination) {
    QSettings settings;
    double treadmill_incline_min = settings.value(QZSettings::treadmill_incline_min, QZSettings::default_treadmill_incline_min).toDouble();
    double treadmill_incline_max = settings.value(QZSettings::treadmill_incline_max, QZSettings::default_treadmill_incline_max).toDouble();

    if(grade < treadmill_incline_min) {
        grade = treadmill_incline_min;
        qDebug() << "grade override due to treadmill_incline_min" << grade;
    } else if(grade > treadmill_incline_max) {
        grade = treadmill_incline_max;
        qDebug() << "grade override due to treadmill_incline_max" << grade;
    }

    m_lastRawInclinationRequested = grade;
    Q_UNUSED(inclination);
    qDebug() << "changeInclination" << grade << autoResistanceEnable << m_inclination_difficult
             << m_inclination_difficult_offset;
    RequestedInclination = (grade * m_inclination_difficult) + m_inclination_difficult_offset;
    if (autoResistanceEnable) {
        requestInclination = (grade * m_inclination_difficult) + m_inclination_difficult_offset;
    }
}
void treadmill::changeSpeedAndInclination(double speed, double inclination) {
    changeSpeed(speed);
    changeInclination(inclination, inclination);
}
metric treadmill::currentInclination() { return Inclination; }
bool treadmill::connected() { return false; }
bluetoothdevice::BLUETOOTH_TYPE treadmill::deviceType() { return bluetoothdevice::TREADMILL; }

double treadmill::minStepInclination() { return 0.5; }
double treadmill::minStepSpeed() { return 0.5; }

void treadmill::update_metrics(bool watt_calc, const double watts) {

    QDateTime current = QDateTime::currentDateTime();
    double deltaTime = (((double)_lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
    QSettings settings;
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();

    simulateInclinationWithSpeed();

    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")) == false &&
        !power_as_treadmill)
        watt_calc = false;

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

            if (Cadence.value() > 0 && instantaneousStrideLengthCMAvailableFromDevice == false) {
                InstantaneousStrideLengthCM = ((Speed.value() / 60.0) * 100000) / Cadence.value();
            }
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

uint16_t treadmill::wattsCalc(double weight, double speed, double inclination) {
    uint16_t watts = 0;
    if (speed > 0) {
        // calc Watts ref. https://alancouzens.com/blog/Run_Power.html
        double pace = 60 / speed;
        double VO2R = 210.0 / pace;
        double VO2A = (VO2R * weight) / 1000.0;
        double hwatts = 75 * VO2A;
        double vwatts = ((9.8 * weight) * (inclination / 100.0));
        watts = hwatts + vwatts;
    }
    return watts;
}

uint16_t treadmill::watts(double weight) {
    if(!powerReceivedFromPowerSensor) {
        uint16_t watts = wattsCalc(weight, currentSpeed().value(), currentInclination().value());
        m_watt.setValue(watts);
    }
    return m_watt.value();
}

void treadmill::clearStats() {

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
    Cadence.clear(false);

    Inclination.clear(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].clear(false);
    }    
}

void treadmill::setPaused(bool p) {

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
    Cadence.setPaused(p);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setPaused(p);
    }    
}

void treadmill::setLap() {

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
    Cadence.setLap(false);

    Inclination.setLap(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setLap(false);
    }
}

void treadmill::setLastSpeed(double speed) { lastSpeed = speed; }

void treadmill::setLastInclination(double inclination) { lastInclination = inclination; }

bool treadmill::autoPauseWhenSpeedIsZero() { return false; }
bool treadmill::autoStartWhenSpeedIsGreaterThenZero() { return false; }

double treadmill::requestedSpeed() { return requestSpeed; }
double treadmill::requestedInclination() { return requestInclination; }
double treadmill::currentTargetSpeed() { return targetSpeed; }

void treadmill::cadenceSensor(uint8_t cadence) { Cadence.setValue(cadence); }
void treadmill::powerSensor(uint16_t power) {
    if(power > 0) {
        powerReceivedFromPowerSensor = true;
        qDebug() << "powerReceivedFromPowerSensor" << powerReceivedFromPowerSensor << power;
    }
    m_watt.setValue(power, false); 
}
void treadmill::speedSensor(double speed) { Speed.setValue(speed); }
void treadmill::instantaneousStrideLengthSensor(double length) { InstantaneousStrideLengthCM.setValue(length); }
void treadmill::groundContactSensor(double groundContact) { GroundContactMS.setValue(groundContact); }
void treadmill::verticalOscillationSensor(double verticalOscillation) {
    VerticalOscillationMM.setValue(verticalOscillation);
}

double treadmill::treadmillInclinationOverrideReverse(double Inclination) {
    for (int i = 0; i <= 15 * 2; i++) {
        if (treadmillInclinationOverride(((double)(i)) / 2.0) <= Inclination &&
            treadmillInclinationOverride(((double)(i + 1)) / 2.0) > Inclination) {
            qDebug() << QStringLiteral("treadmillInclinationOverrideReverse")
                     << treadmillInclinationOverride(((double)(i)) / 2.0)
                     << treadmillInclinationOverride(((double)(i + 1)) / 2.0) << Inclination << i;
            return ((double)i) / 2.0;
        }
    }
    if (Inclination < treadmillInclinationOverride(0))
        return 0;
    else
        return 15;
}

double treadmill::treadmillInclinationOverride(double Inclination) {
    QSettings settings;

    double treadmill_inclination_ovveride_gain = settings
                                                     .value(QZSettings::treadmill_inclination_ovveride_gain,
                                                            QZSettings::default_treadmill_inclination_ovveride_gain)
                                                     .toDouble();
    double treadmill_inclination_ovveride_offset = settings
                                                       .value(QZSettings::treadmill_inclination_ovveride_offset,
                                                              QZSettings::default_treadmill_inclination_ovveride_offset)
                                                       .toDouble();

    Inclination = Inclination * treadmill_inclination_ovveride_gain;
    Inclination = Inclination + treadmill_inclination_ovveride_offset;

    int inc = Inclination * 10;
    qDebug() << "treadmillInclinationOverride" << Inclination << inc;
    switch (inc) {
    case 0:
        return settings
            .value(QZSettings::treadmill_inclination_override_0, QZSettings::default_treadmill_inclination_override_0)
            .toDouble();
    case 5:
        return settings
            .value(QZSettings::treadmill_inclination_override_05, QZSettings::default_treadmill_inclination_override_05)
            .toDouble();
    case 10:
        return settings
            .value(QZSettings::treadmill_inclination_override_10, QZSettings::default_treadmill_inclination_override_10)
            .toDouble();
    case 15:
        return settings
            .value(QZSettings::treadmill_inclination_override_15, QZSettings::default_treadmill_inclination_override_15)
            .toDouble();
    case 20:
        return settings
            .value(QZSettings::treadmill_inclination_override_20, QZSettings::default_treadmill_inclination_override_20)
            .toDouble();
    case 25:
        return settings
            .value(QZSettings::treadmill_inclination_override_25, QZSettings::default_treadmill_inclination_override_25)
            .toDouble();
    case 30:
        return settings
            .value(QZSettings::treadmill_inclination_override_30, QZSettings::default_treadmill_inclination_override_30)
            .toDouble();
    case 35:
        return settings
            .value(QZSettings::treadmill_inclination_override_35, QZSettings::default_treadmill_inclination_override_35)
            .toDouble();
    case 40:
        return settings
            .value(QZSettings::treadmill_inclination_override_40, QZSettings::default_treadmill_inclination_override_40)
            .toDouble();
    case 45:
        return settings
            .value(QZSettings::treadmill_inclination_override_45, QZSettings::default_treadmill_inclination_override_45)
            .toDouble();
    case 50:
        return settings
            .value(QZSettings::treadmill_inclination_override_50, QZSettings::default_treadmill_inclination_override_50)
            .toDouble();
    case 55:
        return settings
            .value(QZSettings::treadmill_inclination_override_55, QZSettings::default_treadmill_inclination_override_55)
            .toDouble();
    case 60:
        return settings
            .value(QZSettings::treadmill_inclination_override_60, QZSettings::default_treadmill_inclination_override_60)
            .toDouble();
    case 65:
        return settings
            .value(QZSettings::treadmill_inclination_override_65, QZSettings::default_treadmill_inclination_override_65)
            .toDouble();
    case 70:
        return settings
            .value(QZSettings::treadmill_inclination_override_70, QZSettings::default_treadmill_inclination_override_70)
            .toDouble();
    case 75:
        return settings
            .value(QZSettings::treadmill_inclination_override_75, QZSettings::default_treadmill_inclination_override_75)
            .toDouble();
    case 80:
        return settings
            .value(QZSettings::treadmill_inclination_override_80, QZSettings::default_treadmill_inclination_override_80)
            .toDouble();
    case 85:
        return settings
            .value(QZSettings::treadmill_inclination_override_85, QZSettings::default_treadmill_inclination_override_85)
            .toDouble();
    case 90:
        return settings
            .value(QZSettings::treadmill_inclination_override_90, QZSettings::default_treadmill_inclination_override_90)
            .toDouble();
    case 95:
        return settings
            .value(QZSettings::treadmill_inclination_override_95, QZSettings::default_treadmill_inclination_override_95)
            .toDouble();
    case 100:
        return settings
            .value(QZSettings::treadmill_inclination_override_100,
                   QZSettings::default_treadmill_inclination_override_100)
            .toDouble();
    case 105:
        return settings
            .value(QZSettings::treadmill_inclination_override_105,
                   QZSettings::default_treadmill_inclination_override_105)
            .toDouble();
    case 110:
        return settings
            .value(QZSettings::treadmill_inclination_override_110,
                   QZSettings::default_treadmill_inclination_override_110)
            .toDouble();
    case 115:
        return settings
            .value(QZSettings::treadmill_inclination_override_115,
                   QZSettings::default_treadmill_inclination_override_115)
            .toDouble();
    case 120:
        return settings
            .value(QZSettings::treadmill_inclination_override_120,
                   QZSettings::default_treadmill_inclination_override_120)
            .toDouble();
    case 125:
        return settings
            .value(QZSettings::treadmill_inclination_override_125,
                   QZSettings::default_treadmill_inclination_override_125)
            .toDouble();
    case 130:
        return settings
            .value(QZSettings::treadmill_inclination_override_130,
                   QZSettings::default_treadmill_inclination_override_130)
            .toDouble();
    case 135:
        return settings
            .value(QZSettings::treadmill_inclination_override_135,
                   QZSettings::default_treadmill_inclination_override_135)
            .toDouble();
    case 140:
        return settings
            .value(QZSettings::treadmill_inclination_override_140,
                   QZSettings::default_treadmill_inclination_override_140)
            .toDouble();
    case 145:
        return settings
            .value(QZSettings::treadmill_inclination_override_145,
                   QZSettings::default_treadmill_inclination_override_145)
            .toDouble();
    case 150:
        return settings
            .value(QZSettings::treadmill_inclination_override_150,
                   QZSettings::default_treadmill_inclination_override_150)
            .toDouble();
    }
    return Inclination;
}

void treadmill::evaluateStepCount() {
    StepCount += (Cadence.lastChanged().msecsTo(QDateTime::currentDateTime())) * (Cadence.value() / 60000);
}

void treadmill::cadenceFromAppleWatch() {
    QSettings settings;
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    if (settings.value(QZSettings::garmin_companion, QZSettings::default_garmin_companion).toBool()) {
        lockscreen h;
        evaluateStepCount();
        Cadence = h.getFootCad();
        qDebug() << QStringLiteral("Current Garmin Cadence: ") << QString::number(Cadence.value());
    } else if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                   .toString()
                   .startsWith(QStringLiteral("Disabled"))) {
        lockscreen h;
        evaluateStepCount();
        long appleWatchCadence = h.stepCadence();
        Cadence = appleWatchCadence;
        qDebug() << QStringLiteral("Current Cadence: ") << QString::number(Cadence.value());
    }
#endif
#endif

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::garmin_companion, QZSettings::default_garmin_companion).toBool()) {
        evaluateStepCount();
        Cadence = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Garmin", "getFootCad", "()I");
        qDebug() << QStringLiteral("Current Garmin Cadence: ") << QString::number(Cadence.value());
    }
#endif    
}

bool treadmill::simulateInclinationWithSpeed() {
    QSettings settings;
    bool treadmill_simulate_inclination_with_speed =
        settings
            .value(QZSettings::treadmill_simulate_inclination_with_speed,
                   QZSettings::default_treadmill_simulate_inclination_with_speed)
            .toBool();
    double w = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    if (treadmill_simulate_inclination_with_speed) {
        if (requestInclination != -100) {
            qDebug() << QStringLiteral("treadmill_simulate_inclination_with_speed enabled!") << requestInclination
                     << requestSpeed << m_lastRawSpeedRequested;
            if (requestSpeed != -1) {
                requestSpeed =
                    wattsCalc(w, requestSpeed, requestInclination) * requestSpeed / wattsCalc(w, requestSpeed, 0);
            } else if (m_lastRawSpeedRequested != -1) {
                requestSpeed = wattsCalc(w, m_lastRawSpeedRequested, requestInclination) * m_lastRawSpeedRequested /
                               wattsCalc(w, m_lastRawSpeedRequested, 0);
            }
        }
        requestInclination = -100;
        return true;
    }
    return false;
}

QTime treadmill::lastRequestedPace() {
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (lastRequestedSpeed().value() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = lastRequestedSpeed().value() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}
