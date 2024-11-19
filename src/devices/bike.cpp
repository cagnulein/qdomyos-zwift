
#include "devices/bike.h"
#include "qdebugfixup.h"
#include "homeform.h"
#include <QSettings>

bike::bike() { elapsed.setType(metric::METRIC_ELAPSED); }

virtualbike *bike::VirtualBike() { return dynamic_cast<virtualbike*>(this->VirtualDevice()); }

void bike::changeResistance(resistance_t resistance) {
    QSettings settings;
    double zwift_erg_resistance_up =
        settings.value(QZSettings::zwift_erg_resistance_up, QZSettings::default_zwift_erg_resistance_up).toDouble();
    double zwift_erg_resistance_down =
        settings.value(QZSettings::zwift_erg_resistance_down, QZSettings::default_zwift_erg_resistance_down).toDouble();

    qDebug() << QStringLiteral("bike::changeResistance") << autoResistanceEnable << resistance;

    lastRawRequestedResistanceValue = resistance;
    if (autoResistanceEnable) {
        double v = (resistance * m_difficult) + gears();
        if ((double)v > zwift_erg_resistance_up) {
            qDebug() << "zwift_erg_resistance_up filter enabled!";
            v = (resistance_t)zwift_erg_resistance_up;
        } else if ((double)v < zwift_erg_resistance_down) {
            qDebug() << "zwift_erg_resistance_down filter enabled!";
            v = (resistance_t)zwift_erg_resistance_down;
        }
        requestResistance = v;
        emit resistanceChanged(requestResistance);
    }
    RequestedResistance = resistance * m_difficult + gears();
}

void bike::changeInclination(double grade, double percentage) {
    qDebug() << QStringLiteral("bike::changeInclination") << autoResistanceEnable << grade << percentage;
    lastRawRequestedInclinationValue = grade;
    if (autoResistanceEnable) {        
        requestInclination = grade;
    }
    emit inclinationChanged(grade, percentage);
}

// originally made for renphobike, but i guess it could be very generic
uint16_t bike::powerFromResistanceRequest(resistance_t requestResistance) {
    // this bike has resistance level to N.m so the formula is Power (kW) = Torque (N.m) x Speed (RPM) / 9.5488
    double cadence = RequestedCadence.value();
    if (cadence <= 0)
        cadence = Cadence.value();
    return (requestResistance * cadence) / 9.5488;
}

void bike::changeRequestedPelotonResistance(int8_t resistance) { RequestedPelotonResistance = resistance; }
void bike::changeCadence(int16_t cadence) { RequestedCadence = cadence; }
void bike::changePower(int32_t power) {

    RequestedPower = power; // in order to paint in any case the request power on the charts

    if (!autoResistanceEnable) {
        qDebug() << QStringLiteral("changePower ignored because auto resistance is disabled");
        return;
    }

    requestPower = power; // used by some bikes that have ERG mode builtin
    QSettings settings;
    bool force_resistance =
        settings.value(QZSettings::virtualbike_forceresistance, QZSettings::default_virtualbike_forceresistance)
            .toBool();
    // bool erg_mode = settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool(); //Not used
    // anywhere in code
    double erg_filter_upper =
        settings.value(QZSettings::zwift_erg_filter, QZSettings::default_zwift_erg_filter).toDouble();
    double erg_filter_lower =
        settings.value(QZSettings::zwift_erg_filter_down, QZSettings::default_zwift_erg_filter_down).toDouble();
    double deltaDown = wattsMetric().value() - ((double)power);
    double deltaUp = ((double)power) - wattsMetric().value();
    qDebug() << QStringLiteral("filter  ") + QString::number(deltaUp) + " " + QString::number(deltaDown) + " " +
                    QString::number(erg_filter_upper) + " " + QString::number(erg_filter_lower);
    if (!ergModeSupported && force_resistance /*&& erg_mode*/ &&
        (deltaUp > erg_filter_upper || deltaDown > erg_filter_lower)) {
        resistance_t r = (resistance_t)resistanceFromPowerRequest(power);
        changeResistance(r); // resistance start from 1
    }
}

double bike::gears() {
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();
    double gears_offset = settings.value(QZSettings::gears_offset, QZSettings::default_gears_offset).toDouble();
    if(gears_zwift_ratio) {
        if(m_gears < 1)
            return 1.0;
        else if(m_gears > 24)
            return 24.0;
    }
    return m_gears + gears_offset;
}
void bike::setGears(double gears) {
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();
    double gears_offset = settings.value(QZSettings::gears_offset, QZSettings::default_gears_offset).toDouble();
    gears -= gears_offset;
    qDebug() << "setGears" << gears;
    if(gears_zwift_ratio && (gears > 24 || gears < 1)) {
        qDebug() << "new gear value ignored because of gears_zwift_ratio setting!";
        return;
    }
    if(gears > maxGears()) {
        qDebug() << "new gear value ignored because of maxGears" << maxGears();
        return;
    }
    if(gears < minGears()) {
        qDebug() << "new gear value ignored because of minGears" << minGears();
        return;
    }
    m_gears = gears;
    if(homeform::singleton()) {
        homeform::singleton()->updateGearsValue();
    }
    if (settings.value(QZSettings::gears_restore_value, QZSettings::default_gears_restore_value).toBool())
        settings.setValue(QZSettings::gears_current_value, m_gears);
    if (lastRawRequestedResistanceValue != -1) {
        changeResistance(lastRawRequestedResistanceValue);
    }
}

double bike::currentCrankRevolutions() { return CrankRevs; }
uint16_t bike::lastCrankEventTime() { return LastCrankEventTime; }
metric bike::lastRequestedResistance() { return RequestedResistance; }
metric bike::lastRequestedPelotonResistance() { return RequestedPelotonResistance; }
metric bike::lastRequestedCadence() { return RequestedCadence; }
metric bike::lastRequestedPower() { return RequestedPower; }
metric bike::currentResistance() { return Resistance; }
uint8_t bike::fanSpeed() { return FanSpeed; }
bool bike::connected() { return false; }
uint16_t bike::watts() { return 0; }
metric bike::pelotonResistance() { return m_pelotonResistance; }
resistance_t bike::pelotonToBikeResistance(int pelotonResistance) { return pelotonResistance; }
resistance_t bike::resistanceFromPowerRequest(uint16_t power) { return power / 10; } // in order to have something
void bike::cadenceSensor(uint8_t cadence) { Cadence.setValue(cadence); }
void bike::powerSensor(uint16_t power) { m_watt.setValue(power, false); }

bluetoothdevice::BLUETOOTH_TYPE bike::deviceType() { return bluetoothdevice::BIKE; }

void bike::clearStats() {

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

    RequestedPelotonResistance.clear(false);
    RequestedResistance.clear(false);
    RequestedCadence.clear(false);
    RequestedPower.clear(false);
    m_pelotonResistance.clear(false);
    Cadence.clear(false);
    Resistance.clear(false);
    WattKg.clear(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].clear(false);
    }    
}

void bike::setPaused(bool p) {

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
    WattKg.setPaused(p);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setPaused(p);
    }    
}

void bike::setLap() {

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

    RequestedPelotonResistance.setLap(false);
    RequestedResistance.setLap(false);
    RequestedCadence.setLap(false);
    RequestedPower.setLap(false);
    m_pelotonResistance.setLap(false);
    Cadence.setLap(false);
    Resistance.setLap(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setLap(false);
    }    
}

uint8_t bike::metrics_override_heartrate() {

    QSettings settings;
    QString setting =
        settings.value(QZSettings::peloton_heartrate_metric, QZSettings::default_peloton_heartrate_metric).toString();
    if (!setting.compare(QStringLiteral("Heart Rate"))) {
        return qRound(currentHeart().value());
    } else if (!setting.compare(QStringLiteral("Speed"))) {

        return qRound(currentSpeed().value());
    } else if (!setting.compare(QStringLiteral("Inclination"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Cadence"))) {

        return qRound(Cadence.value());
    } else if (!setting.compare(QStringLiteral("Elevation"))) {

        return qRound(elevationGain().value());
    } else if (!setting.compare(QStringLiteral("Calories"))) {

        return qRound(calories().value());
    } else if (!setting.compare(QStringLiteral("Odometer"))) {

        return qRound(odometer());
    } else if (!setting.compare(QStringLiteral("Pace"))) {

        return currentPace().second();
    } else if (!setting.compare(QStringLiteral("Resistance"))) {

        return qRound(Resistance.value());
    } else if (!setting.compare(QStringLiteral("Watt"))) {

        return qRound(wattsMetric().value());
    } else if (!setting.compare(QStringLiteral("Weight Loss"))) {

        return qRound(weightLoss());
    } else if (!setting.compare(QStringLiteral("AVG Watt"))) {

        return qRound(wattsMetric().average());
    } else if (!setting.compare(QStringLiteral("FTP"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Fan"))) {

        return FanSpeed;
    } else if (!setting.compare(QStringLiteral("Jouls"))) {

        return qRound(jouls().value());
    } else if (!setting.compare(QStringLiteral("Lap Elapsed"))) {

        return lapElapsedTime().second();
    } else if (!setting.compare(QStringLiteral("Elapsed"))) {

        return qRound(elapsed.value());
    } else if (!setting.compare(QStringLiteral("Moving Time"))) {

        return movingTime().second();
    } else if (!setting.compare(QStringLiteral("Peloton Offset"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Peloton Resistance"))) {

        return qRound(pelotonResistance().value());
    } else if (!setting.compare(QStringLiteral("Date Time"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Target Resistance"))) {

        return qRound(RequestedResistance.value());
    } else if (!setting.compare(QStringLiteral("Target Peloton Resistance"))) {

        return qRound(RequestedPelotonResistance.value());
    } else if (!setting.compare(QStringLiteral("Target Power"))) {

        return qRound(RequestedPower.value());
    } else if (!setting.compare(QStringLiteral("Watt/Kg"))) {
        return qRound(wattKg().value());
    } else if (!setting.compare(QStringLiteral("Target Cadence"))) {
        return qRound(RequestedCadence.value());
    }
    return qRound(currentHeart().value());
}

bool bike::inclinationAvailableByHardware() { return false; }

uint16_t bike::wattFromHR(bool useSpeedAndCadence) {
    QSettings settings;
    double watt = 0;
    if (currentCadence().value() == 0 && useSpeedAndCadence == true) {
        return 0;
    }
    if (Heart.value() > 0) {
        int avgP = ((settings.value(QZSettings::power_hr_pwr1, QZSettings::default_power_hr_pwr1).toDouble() *
                     settings.value(QZSettings::power_hr_hr2, QZSettings::default_power_hr_hr2).toDouble()) -
                    (settings.value(QZSettings::power_hr_pwr2, QZSettings::default_power_hr_pwr2).toDouble() *
                     settings.value(QZSettings::power_hr_hr1, QZSettings::default_power_hr_hr1).toDouble())) /
                       (settings.value(QZSettings::power_hr_hr2, QZSettings::default_power_hr_hr2).toDouble() -
                        settings.value(QZSettings::power_hr_hr1, QZSettings::default_power_hr_hr1).toDouble()) +
                   (Heart.value() *
                    ((settings.value(QZSettings::power_hr_pwr1, QZSettings::default_power_hr_pwr1).toDouble() -
                      settings.value(QZSettings::power_hr_pwr2, QZSettings::default_power_hr_pwr2).toDouble()) /
                     (settings.value(QZSettings::power_hr_hr1, QZSettings::default_power_hr_hr1).toDouble() -
                      settings.value(QZSettings::power_hr_hr2, QZSettings::default_power_hr_hr2).toDouble())));
        if (Speed.value() > 0 || useSpeedAndCadence == false) {
            if (avgP < 50) {
                avgP = 50;
            }
            watt = avgP;
        } else {
            watt = 0;
        }
    } else {
        watt = currentCadence().value() * 1.2; // random value cloned from Zwift when HR is not available
    }
    return watt;
}

double bike::gearsZwiftRatio() {
    if(m_gears <= 0)
        return 0.65;
    else if(m_gears > 24)
        return 6;
    switch((int)m_gears) {
        case 1:
            return 0.75;
        case 2:
            return 0.87;
        case 3:
            return 0.99;
        case 4:
            return 1.11;
        case 5:
            return 1.23;
        case 6:
            return 1.38;
        case 7:
            return 1.53;
        case 8:
            return 1.68;
        case 9:
            return 1.86;
        case 10:
            return 2.04;
        case 11:
            return 2.22;
        case 12:
            return 2.40;
        case 13:
            return 2.61;
        case 14:
            return 2.82;
        case 15:
            return 3.03;
        case 16:
            return 3.24;
        case 17:
            return 3.49;
        case 18:
            return 3.74;
        case 19:
            return 3.99;
        case 20:
            return 4.24;
        case 21:
            return 4.54;
        case 22:
            return 4.84;
        case 23:
            return 5.14;
        case 24:
            return 5.49;                        
    }
    return 1;
}
