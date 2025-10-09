#include "treadmill.h"
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif
#include <QSettings>

treadmill::treadmill() {}

void treadmill::changeSpeed(double speed) {
    // Reset target watts only if called from external source
    if (!callingFromFollowPower) {
        targetWatts = -1;
        qDebug() << "External speed change - resetting power following mode";
    }
    
    QSettings settings;
    bool stryd_speed_instead_treadmill = settings.value(QZSettings::stryd_speed_instead_treadmill, QZSettings::default_stryd_speed_instead_treadmill).toBool();
    m_lastRawSpeedRequested = speed;
    speed /= settings.value(QZSettings::speed_gain, QZSettings::default_speed_gain).toDouble();
    speed -= settings.value(QZSettings::speed_offset, QZSettings::default_speed_offset).toDouble();    
    if(stryd_speed_instead_treadmill && Speed.value() > 0) {
        double delta = (Speed.value() - rawSpeed.value());
        double maxAllowedDelta = speed * 0.20; // 20% of the speed request

        if (std::abs(delta) <= maxAllowedDelta) {
            qDebug() << "stryd_speed_instead_treadmill so override speed by " << delta;
            speed -= delta;
        } else {
            qDebug() << "Delta" << delta << "exceeds 20% threshold of" << maxAllowedDelta << "- not applying correction";
        }
    }
    qDebug() << "changeSpeed" << speed << autoResistanceEnable << m_difficult << m_difficult_offset << m_lastRawSpeedRequested;
    RequestedSpeed = (speed * m_difficult) + m_difficult_offset;
    if (autoResistanceEnable)
        requestSpeed = (speed * m_difficult) + m_difficult_offset;
}
void treadmill::changeInclination(double grade, double inclination) {    
    QSettings settings;
    double treadmill_incline_min = settings.value(QZSettings::treadmill_incline_min, QZSettings::default_treadmill_incline_min).toDouble();
    double treadmill_incline_max = settings.value(QZSettings::treadmill_incline_max, QZSettings::default_treadmill_incline_max).toDouble();
    double step = settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline).toDouble();
    bool stryd_inclination_instead_treadmill = settings.value(QZSettings::stryd_inclination_instead_treadmill, QZSettings::default_stryd_inclination_instead_treadmill).toBool();

    if(grade < treadmill_incline_min) {
        grade = treadmill_incline_min;
        qDebug() << "grade override due to treadmill_incline_min" << grade;
    } else if(grade > treadmill_incline_max) {
        grade = treadmill_incline_max;
        qDebug() << "grade override due to treadmill_incline_max" << grade;
    }

    if(stryd_inclination_instead_treadmill) {
        double delta = (Inclination.value() - rawInclination.value());
        double maxAllowedDelta = grade * 0.20; // 20% of the inclination request

        if (std::abs(delta) <= maxAllowedDelta) {
            qDebug() << "stryd_inclination_instead_treadmill so override inclination by " << delta;
            grade -= delta;
        } else {
            qDebug() << "Delta" << delta << "exceeds 20% threshold of" << maxAllowedDelta << "- not applying correction";
        }
    }

    m_lastRawInclinationRequested = grade;
    Q_UNUSED(inclination);
    qDebug() << "changeInclination" << grade << autoResistanceEnable << m_inclination_difficult
             << m_inclination_difficult_offset;
    
    // Calculate the raw requested inclination
    double rawInclination = (grade * m_inclination_difficult) + m_inclination_difficult_offset;
    
    // Round to nearest step
    RequestedInclination = round(rawInclination / step) * step;
    
    if (autoResistanceEnable) {
        requestInclination = RequestedInclination.value();  // Use the rounded value here as well
    }
}
void treadmill::changeSpeedAndInclination(double speed, double inclination) {
    changeSpeed(speed);
    changeInclination(inclination, inclination);
}
metric treadmill::currentInclination() { return Inclination; }
bool treadmill::connected() { return false; }
BLUETOOTH_TYPE treadmill::deviceType() { return TREADMILL; }

double treadmill::minStepInclination() { return 0.5; }
double treadmill::minStepSpeed() { return 0.5; }

void treadmill::update_metrics(bool watt_calc, const double watts, const bool from_accessory) {

    QDateTime current = QDateTime::currentDateTime();
    double deltaTime = (((double)_lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
    QSettings settings;
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();

    simulateInclinationWithSpeed();
    if(!from_accessory)
        followPowerBySpeed();

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
    if (currentInclination().value() > 0)
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
    rawSpeed.clear(false);
    rawInclination.clear(false);
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
    rawSpeed.setPaused(p);
    rawInclination.setPaused(p);
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
    rawSpeed.setLap(false);
    rawInclination.setLap(false);
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
    double vwatts = 0;
    if(power > 0) {
        powerReceivedFromPowerSensor = true;
        qDebug() << "powerReceivedFromPowerSensor" << powerReceivedFromPowerSensor << power;
        QSettings settings;
        if(currentInclination().value() != 0 && settings.value(QZSettings::stryd_add_inclination_gain, QZSettings::default_stryd_add_inclination_gain).toBool()) {
            QSettings settings;
            double w = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
            // calc Watts ref. https://alancouzens.com/blog/Run_Power.html
            vwatts = ((9.8 * w) * (currentInclination().value() / 100.0));
            qDebug() << QStringLiteral("overrding power read from the sensor of ") << power << QStringLiteral("with ") << vwatts << QStringLiteral(" for the treadmill inclination");
        }
    }
    m_watt.setValue(power + vwatts, false); 
}

void treadmill::speedSensor(double speed) {
    Speed.setValue(speed);
    qDebug() << "Current speed: " << speed;
}

void treadmill::inclinationSensor(double grade, double inclination) {
    Inclination.setValue(inclination);
    qDebug() << "Current Inclination: " << inclination;
}

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

    // if the inclination is negative, since the table consider only positive values, I return the actual value
    if(Inclination < 0)
        return Inclination;
    else if (Inclination < treadmillInclinationOverride(0))
        return treadmillInclinationOverride(0);
    else
        return treadmillInclinationOverride(15);
}

bool treadmill::areInclinationSettingsDefault() {
    QSettings settings;
    
    // Check gain and offset settings first
    if (settings.value(QZSettings::treadmill_inclination_ovveride_gain).toDouble() !=
        QZSettings::default_treadmill_inclination_ovveride_gain) {
        return false;
    }
    
    if (settings.value(QZSettings::treadmill_inclination_ovveride_offset).toDouble() !=
        QZSettings::default_treadmill_inclination_ovveride_offset) {
        return false;
    }
    
    // Array of settings to check - pairs of setting key and its default value
    const struct {
        QString setting;
        double defaultValue;
    } checkPairs[] = {
        {QZSettings::treadmill_inclination_override_0, QZSettings::default_treadmill_inclination_override_0},
        {QZSettings::treadmill_inclination_override_05, QZSettings::default_treadmill_inclination_override_05},
        {QZSettings::treadmill_inclination_override_10, QZSettings::default_treadmill_inclination_override_10},
        {QZSettings::treadmill_inclination_override_15, QZSettings::default_treadmill_inclination_override_15},
        {QZSettings::treadmill_inclination_override_20, QZSettings::default_treadmill_inclination_override_20},
        {QZSettings::treadmill_inclination_override_25, QZSettings::default_treadmill_inclination_override_25},
        {QZSettings::treadmill_inclination_override_30, QZSettings::default_treadmill_inclination_override_30},
        {QZSettings::treadmill_inclination_override_35, QZSettings::default_treadmill_inclination_override_35},
        {QZSettings::treadmill_inclination_override_40, QZSettings::default_treadmill_inclination_override_40},
        {QZSettings::treadmill_inclination_override_45, QZSettings::default_treadmill_inclination_override_45},
        {QZSettings::treadmill_inclination_override_50, QZSettings::default_treadmill_inclination_override_50},
        {QZSettings::treadmill_inclination_override_55, QZSettings::default_treadmill_inclination_override_55},
        {QZSettings::treadmill_inclination_override_60, QZSettings::default_treadmill_inclination_override_60},
        {QZSettings::treadmill_inclination_override_65, QZSettings::default_treadmill_inclination_override_65},
        {QZSettings::treadmill_inclination_override_70, QZSettings::default_treadmill_inclination_override_70},
        {QZSettings::treadmill_inclination_override_75, QZSettings::default_treadmill_inclination_override_75},
        {QZSettings::treadmill_inclination_override_80, QZSettings::default_treadmill_inclination_override_80},
        {QZSettings::treadmill_inclination_override_85, QZSettings::default_treadmill_inclination_override_85},
        {QZSettings::treadmill_inclination_override_90, QZSettings::default_treadmill_inclination_override_90},
        {QZSettings::treadmill_inclination_override_95, QZSettings::default_treadmill_inclination_override_95},
        {QZSettings::treadmill_inclination_override_100, QZSettings::default_treadmill_inclination_override_100},
        {QZSettings::treadmill_inclination_override_105, QZSettings::default_treadmill_inclination_override_105},
        {QZSettings::treadmill_inclination_override_110, QZSettings::default_treadmill_inclination_override_110},
        {QZSettings::treadmill_inclination_override_115, QZSettings::default_treadmill_inclination_override_115},
        {QZSettings::treadmill_inclination_override_120, QZSettings::default_treadmill_inclination_override_120},
        {QZSettings::treadmill_inclination_override_125, QZSettings::default_treadmill_inclination_override_125},
        {QZSettings::treadmill_inclination_override_130, QZSettings::default_treadmill_inclination_override_130},
        {QZSettings::treadmill_inclination_override_135, QZSettings::default_treadmill_inclination_override_135},
        {QZSettings::treadmill_inclination_override_140, QZSettings::default_treadmill_inclination_override_140},
        {QZSettings::treadmill_inclination_override_145, QZSettings::default_treadmill_inclination_override_145},
        {QZSettings::treadmill_inclination_override_150, QZSettings::default_treadmill_inclination_override_150}
    };
    
    // Check each setting against its default value
    for (const auto& pair : checkPairs) {
        if (settings.value(pair.setting).toDouble() != pair.defaultValue) {
            return false;
        }
    }
    
    // If we got here, all settings match their defaults
    return true;
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
    StepCount += (Cadence.lastChanged().msecsTo(QDateTime::currentDateTime())) * (Cadence.value() / 60000) * 2.0;
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
        Cadence = QJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Garmin", "getFootCad", "()I");
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

bool treadmill::followPowerBySpeed() {
    QSettings settings;
    bool r = false;
    bool treadmill_follow_wattage =
        settings
            .value(QZSettings::treadmill_follow_wattage,
                   QZSettings::default_treadmill_follow_wattage)
            .toBool();
    double w = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    static double lastInclination = 0;

    if (treadmill_follow_wattage) {

        if (currentInclination().value() != lastInclination && wattsMetric().value() != 0) {
            
            // If not following power mode, calculate new target from current values
            if (targetWatts == -1) {
                targetWatts = wattsCalc(w, currentSpeed().value(), lastInclination);
                qDebug() << "Starting power following mode with target watts:" << targetWatts;
            }
            
            // Find speed to maintain targetWatts with current inclination
            double newspeed = 0;
            double bestSpeed = 0.1;
            double bestDifference = fabs(wattsCalc(w, bestSpeed, currentInclination().value()) - targetWatts);
            
            for (int speed = 1; speed <= 300; speed++) {
                double s = ((double)speed) / 10.0;
                double thisDifference = fabs(wattsCalc(w, s, currentInclination().value()) - targetWatts);
                if (thisDifference < bestDifference) {
                    bestDifference = thisDifference;
                    bestSpeed = s;
                }
            }
            
            newspeed = bestSpeed;
            qDebug() << "Following power: changing speed to" << newspeed << "to maintain" << targetWatts << "watts (inclination changed" << currentInclination().value() << lastInclination << ")";
            
            callingFromFollowPower = true;  // Set flag before calling
            changeSpeedAndInclination(newspeed, currentInclination().value());
            callingFromFollowPower = false; // Reset flag after calling
            
            r = true;
        }
    }

    lastInclination = currentInclination().value();

    return r;
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

void treadmill::parseInclination(double inclination) {
    QSettings settings;
    bool stryd_inclination_instead_treadmill = settings.value(QZSettings::stryd_inclination_instead_treadmill, QZSettings::default_stryd_inclination_instead_treadmill).toBool();
    if(!stryd_inclination_instead_treadmill) {
        Inclination = inclination;
    } else {
        qDebug() << "Inclination from the treadmill is discarded since we are using the one from the power sensor " << inclination;
    }
    rawInclination = inclination;
}

void treadmill::parseSpeed(double speed) {
    QSettings settings;
    bool stryd_speed_instead_treadmill = settings.value(QZSettings::stryd_speed_instead_treadmill, QZSettings::default_stryd_speed_instead_treadmill).toBool();
    if(!stryd_speed_instead_treadmill) {
        Speed = speed;
    } else {
        qDebug() << "speed from the treadmill is discarded since we are using the one from the power sensor " << speed;
    }
    rawSpeed = speed;
}

/*
 * Running Stress Score
 */
double treadmill::runningStressScore() {
    QSettings settings;
    double sec = this->elapsed.value();
    double NP = this->m_watt.average();
    double CP = settings.value(QZSettings::ftp_run, QZSettings::default_ftp_run).toDouble();
    double part1 = ((((sec) * NP * (NP / CP)) / (CP * 3600) * 100) * 0.6139);
    double part2 = ((((sec) * NP * (NP / CP)) / (CP * 3600) * 100));
    return (part1 + part2) / 2;
}

void treadmill::changePower(int32_t power) {

    RequestedPower = power; // in order to paint in any case the request power on the charts

    if (!autoResistanceEnable) {
        qDebug() << QStringLiteral("changePower ignored because auto resistance is disabled");
        return;
    }

    requestPower = power; // used by some bikes that have ERG mode builtin
    QSettings settings;
    /*
    double erg_filter_upper =
        settings.value(QZSettings::zwift_erg_filter, QZSettings::default_zwift_erg_filter).toDouble();
    double erg_filter_lower =
        settings.value(QZSettings::zwift_erg_filter_down, QZSettings::default_zwift_erg_filter_down).toDouble();
    double deltaDown = wattsMetric().value() - ((double)power);
    double deltaUp = ((double)power) - wattsMetric().value();
    qDebug() << QStringLiteral("filter  ") + QString::number(deltaUp) + " " + QString::number(deltaDown) + " " +
                    QString::number(erg_filter_upper) + " " + QString::number(erg_filter_lower);
    if (!ergModeSupported && force_resistance &&
        (deltaUp > erg_filter_upper || deltaDown > erg_filter_lower)) {
        resistance_t r = (resistance_t)resistanceFromPowerRequest(power);
        changeResistance(r); // resistance start from 1
    }*/

    QString data = settings.value(QZSettings::treadmillDataPoints, QZSettings::default_treadmillDataPoints).toString();
    bool ergTable = data.length() && settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                                             .toString()
                                             .startsWith(QStringLiteral("Disabled")) == false;

    double weightKg = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    double lowSpeed = 0.0; // minimum possible speed
    double highSpeed = 30.0; // some maximum speed that is reasonably not exceeded
    const double tolerance = 3; // acceptable error in watts to stop the search
    const int maxIterations = 300; // maximum number of iterations to prevent infinite loops
    int i = 0;

    for (i = 1; i < maxIterations; i++) {
        double midSpeed = ((double)i / 10.0);
        double calculatedWatts;
        if(ergTable) {
            calculatedWatts = _ergTable.estimateWattage(midSpeed, currentInclination().value());
        } else {
            calculatedWatts = wattsCalc(weightKg, midSpeed, currentInclination().value());
        }

        if (std::abs(calculatedWatts - power) <= tolerance) {
            changeSpeed(midSpeed);
            return;
        }
    }

    if(ergTable && i == maxIterations) {
        lowSpeed = 0.0; // minimum possible speed
        highSpeed = 30.0; // some maximum speed that is reasonably not exceeded
        for (i = 0; i < maxIterations; i++) {
            double midSpeed = (lowSpeed + highSpeed) / 2;
            double calculatedWatts = wattsCalc(weightKg, midSpeed, currentInclination().value());

            if (std::abs(calculatedWatts - power) <= tolerance) {
                changeSpeed(midSpeed);
                return;
            }

            if (calculatedWatts < power) {
                lowSpeed = midSpeed;
            } else {
                highSpeed = midSpeed;
            }
        }
    }

    changeSpeed((lowSpeed + highSpeed) / 2); // Return the best estimate
}

metric treadmill::lastRequestedPower() { return RequestedPower; }

QTime treadmill::speedToPace(double Speed) {
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (Speed == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

