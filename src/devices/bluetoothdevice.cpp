#include "devices/bluetoothdevice.h"

#include <QFile>
#include <QSettings>
#include <QTime>

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#endif
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

bluetoothdevice::bluetoothdevice() {}

bluetoothdevice::~bluetoothdevice() {
    if(this->virtualDevice) {
        delete this->virtualDevice;
        this->virtualDevice = nullptr;
    }
}

BLUETOOTH_TYPE bluetoothdevice::deviceType() { return UNKNOWN; }
void bluetoothdevice::start() { requestStart = 1; lastStart = QDateTime::currentMSecsSinceEpoch(); }
void bluetoothdevice::stop(bool pause) {
    if (pause) {
        // Solo pause, NON stop
        requestPause = 1;
    } else {
        // Stop completo
        requestStop = 1;
    }
}
metric bluetoothdevice::currentHeart() { return Heart; }
metric bluetoothdevice::currentSpeed() { return Speed; }
metric bluetoothdevice::currentInclination() { return Inclination; }
QTime bluetoothdevice::movingTime() {
    int hours = (int)(moving.value() / 3600.0);
    return QTime(hours, (int)(moving.value() - ((double)hours * 3600.0)) / 60.0, ((uint32_t)moving.value()) % 60, 0);
}
QTime bluetoothdevice::elapsedTime() {
    int hours = (int)(elapsed.value() / 3600.0);
    return QTime(hours, (int)(elapsed.value() - ((double)hours * 3600.0)) / 60.0, ((uint32_t)elapsed.value()) % 60, 0);
}
QTime bluetoothdevice::lapElapsedTime() {
    int hours = (int)(elapsed.lapValue() / 3600.0);
    return QTime(hours, (int)(elapsed.lapValue() - ((double)hours * 3600.0)) / 60.0,
                 ((uint32_t)elapsed.lapValue()) % 60, 0);
}

metric bluetoothdevice::currentResistance() { return Resistance; }
metric bluetoothdevice::currentCadence() { return Cadence; }
double bluetoothdevice::currentCrankRevolutions() { return 0; }
uint16_t bluetoothdevice::lastCrankEventTime() { return 0; }

virtualdevice *bluetoothdevice::VirtualDevice() { return this->virtualDevice; }
void bluetoothdevice::changeResistance(resistance_t resistance) {}
void bluetoothdevice::changePower(int32_t power) {}
void bluetoothdevice::changeInclination(double grade, double percentage) {}

void bluetoothdevice::offsetElapsedTime(int offset) { elapsed += offset; }

QTime bluetoothdevice::currentPace() {
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (Speed.value() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.value() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

QTime bluetoothdevice::averagePace() {

    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (Speed.average() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.average() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

QTime bluetoothdevice::maxPace() {

    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    if (miles) {
        unit_conversion = 0.621371;
    }
    if (Speed.max() == 0) {
        return QTime(0, 0, 0, 0);
    } else {
        double speed = Speed.max() * unit_conversion;
        return QTime(0, (int)(1.0 / (speed / 60.0)),
                     (((double)(1.0 / (speed / 60.0)) - ((double)((int)(1.0 / (speed / 60.0))))) * 60.0), 0);
    }
}

double bluetoothdevice::odometerFromStartup() { return Distance.valueRaw(); }
double bluetoothdevice::odometer() { return Distance.value(); }
double bluetoothdevice::lapOdometer() { return Distance.lapValue(); }
metric bluetoothdevice::calories() { 
    QSettings settings;
    bool activeOnly = settings.value(QZSettings::calories_active_only, QZSettings::default_calories_active_only).toBool();
    bool fromHR = settings.value(QZSettings::calories_from_hr, QZSettings::default_calories_from_hr).toBool();
    
    if (fromHR && Heart.value() > 0) {
        // Calculate calories based on heart rate
        double totalHRKCal = metric::calculateKCalfromHR(Heart.average(), elapsed.value());
        hrKCal.setValue(totalHRKCal);
        
        if (activeOnly) {
            activeKCal.setValue(metric::calculateActiveKCal(hrKCal.value(), elapsed.value()));
            return activeKCal;
        } else {
            return hrKCal;
        }
    } else {
        // Power-based calculation (current behavior)
        if (activeOnly) {
            activeKCal.setValue(metric::calculateActiveKCal(KCal.value(), elapsed.value()));
            return activeKCal;
        } else {
            return KCal; 
        }
    }
}

metric bluetoothdevice::totalCalories() { 
    QSettings settings;
    bool fromHR = settings.value(QZSettings::calories_from_hr, QZSettings::default_calories_from_hr).toBool();
    
    if (fromHR && Heart.value() > 0) {
        return hrKCal; // Return HR-based total calories
    } else {
        return KCal;   // Return power-based total calories
    }
}

metric bluetoothdevice::activeCalories() {
    return activeKCal;
}

metric bluetoothdevice::hrCalories() {
    return hrKCal;
}

metric bluetoothdevice::jouls() { return m_jouls; }
uint8_t bluetoothdevice::fanSpeed() { return FanSpeed; };
bool bluetoothdevice::changeFanSpeed(uint8_t speed) {
    // managing underflow
    if (speed > 230 && FanSpeed < 20) {
        speed = 0;
    } else if (speed < 20 && FanSpeed > 230) {
        // managing overflow
        speed = 100;
    }
    if (speed > 100)
        speed = 100;
    // this is useful when there is a fitmetria fanfit in order to set the current
    // value to the last requested
    FanSpeed = speed;

    requestFanSpeed = speed;
    emit fanSpeedChanged(speed);
    return false;
}
bool bluetoothdevice::connected() { return false; }
metric bluetoothdevice::elevationGain() { return elevationAcc; }
void bluetoothdevice::heartRate(uint8_t heart) {
    Heart.setValue(heart);
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    // Write heart rate from Bluetooth to Apple Health during workout
    lockscreen h;
    if(heart > 0)
        h.setHeartRate(heart);
#endif
#endif
}
void bluetoothdevice::coreBodyTemperature(double coreBodyTemperature) { CoreBodyTemperature.setValue(coreBodyTemperature); }
void bluetoothdevice::skinTemperature(double skinTemperature) { SkinTemperature.setValue(skinTemperature); }
void bluetoothdevice::heatStrainIndex(double heatStrainIndex) { HeatStrainIndex.setValue(heatStrainIndex); }
void bluetoothdevice::disconnectBluetooth() {
    if (m_control) {
        m_control->disconnectFromDevice();
    }
}
metric bluetoothdevice::wattsMetric() { return m_watt; }
void bluetoothdevice::setDifficult(double d) { m_difficult = d; }
double bluetoothdevice::difficult() { return m_difficult; }
void bluetoothdevice::setInclinationDifficult(double d) { m_inclination_difficult = d; }
double bluetoothdevice::inclinationDifficult() { return m_inclination_difficult; }
void bluetoothdevice::setDifficultOffset(double d) { m_difficult_offset = d; }
double bluetoothdevice::difficultOffset() { return m_difficult_offset; }
void bluetoothdevice::setInclinationDifficultOffset(double d) { m_inclination_difficult_offset = d; }
double bluetoothdevice::inclinationDifficultOffset() { return m_inclination_difficult_offset; }
void bluetoothdevice::cadenceSensor(uint8_t cadence) { Q_UNUSED(cadence) }
void bluetoothdevice::powerSensor(uint16_t power) { Q_UNUSED(power) }
void bluetoothdevice::speedSensor(double speed) { Q_UNUSED(speed) }
void bluetoothdevice::inclinationSensor(double grade, double inclination) { Q_UNUSED(grade); Q_UNUSED(inclination) }
void bluetoothdevice::instantaneousStrideLengthSensor(double length) { Q_UNUSED(length); }
void bluetoothdevice::groundContactSensor(double groundContact) { Q_UNUSED(groundContact); }
void bluetoothdevice::verticalOscillationSensor(double verticalOscillation) { Q_UNUSED(verticalOscillation); }

bool bluetoothdevice::hasVirtualDevice() { return this->virtualDevice!=nullptr; }

double bluetoothdevice::calculateMETS() { return ((0.048 * m_watt.value()) + 1.19); }

void bluetoothdevice::setVirtualDevice(virtualdevice *virtualDevice, VIRTUAL_DEVICE_MODE mode) {

    if(mode!=VIRTUAL_DEVICE_MODE::NONE && !virtualDevice)
        throw "Virtual device mode should be NONE when no virtual device is specified.";

    if(this->virtualDevice)
        delete this->virtualDevice;
    this->virtualDevice = virtualDevice;
    this->virtualDeviceMode=mode;
}

// keiser m3i has a separate management of this, so please check it
void bluetoothdevice::update_metrics(bool watt_calc, const double watts, const bool from_accessory) {

    QDateTime current = QDateTime::currentDateTime();
    double deltaTime = (((double)_lastTimeUpdate.msecsTo(current)) / ((double)1000.0));
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool power_as_bike =
        settings.value(QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike).toBool();
    bool power_as_treadmill =
        settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();

    if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")) == false &&
        !power_as_bike && !power_as_treadmill)
        watt_calc = false;

    if(deviceType() == BIKE && !from_accessory)  // append only if it's coming from the bike, not from the power sensor
        _ergTable.collectData(Cadence.value(), m_watt.value(), Resistance.value());

    if (!_firstUpdate && !paused) {
        if (currentSpeed().value() > 0.0 || settings.value(QZSettings::continuous_moving, true).toBool()) {

            elapsed += deltaTime;
        }
        if (currentSpeed().value() > 0.0) {

            moving += deltaTime;
            m_jouls += (m_watt.value() * deltaTime);
            WeightLoss = metric::calculateWeightLoss(KCal.value());
            if (watt_calc) {
                m_watt = watts;
            }
            WattKg = m_watt.value() / settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
        } else if (m_watt.value() > 0) {

            if (watt_calc) {
                m_watt = 0;
            }
            WattKg = 0;
        }
    } else if (paused && settings.value(QZSettings::instant_power_on_pause, QZSettings::default_instant_power_on_pause)
                             .toBool()) {
        // useful for FTP test
        if (watt_calc) {
            m_watt = watts;
        }
        WattKg = m_watt.value() / settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    } else if (m_watt.value() > 0) {

        m_watt = 0;
        WattKg = 0;
    }
    METS = calculateMETS();
    if (currentInclination().value() > 0)
        elevationAcc += (currentSpeed().value() / 3600.0) * 1000.0 * (currentInclination().value() / 100.0) * deltaTime;

    _lastTimeUpdate = current;
    _firstUpdate = false;

    // Update iOS Live Activity with throttling
    update_ios_live_activity();
}

void bluetoothdevice::update_hr_from_external() {
    QSettings settings;
    if(settings.value(QZSettings::garmin_companion, QZSettings::default_garmin_companion).toBool()) {
#ifdef Q_OS_ANDROID
        Heart = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/Garmin", "getHR", "()I");
#endif
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        lockscreen h;
        Heart = h.getHR();
#endif
#endif
        qDebug() << "Garmin Companion Heart:" << Heart.value();
    } else {
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen h;
            long appleWatchHeartRate = h.heartRate();
            QSettings settings;
            bool activeOnly = settings.value(QZSettings::calories_active_only, QZSettings::default_calories_active_only).toBool();
            
            if (activeOnly) {
                // When active calories setting is enabled, send both total and active calories
                h.setKcal(calories().value()); // This will be active calories
                h.setTotalKcal(totalCalories().value()); // This will be total calories
            } else {
                // When disabled, send total calories as before
                h.setKcal(calories().value()); // This will be total calories
            }
            h.setDistance(Distance.value());
            h.setSpeed(Speed.value());
            h.setPower(m_watt.value());
            h.setCadence(Cadence.value());
            h.setSteps(StepCount.value());
            Heart = appleWatchHeartRate;
            qDebug() << "Current Heart from Apple Watch: " << QString::number(appleWatchHeartRate);
#endif
#endif
#ifdef Q_OS_ANDROID
    if (!settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        Heart = QAndroidJniObject::callStaticMethod<jint>("org/cagnulen/qdomyoszwift/WearableController", "getHeart", "()I");
        qDebug() << "WearOS Companion Heart:" << Heart.value();
    }
#endif
    }
    // Note: workoutTrackingUpdate is now called from update_ios_live_activity() with throttling
}

void bluetoothdevice::update_ios_live_activity() {
    #ifdef Q_OS_IOS
    #ifndef IO_UNDER_QT
    static QDateTime lastUpdate;
    QDateTime current = QDateTime::currentDateTime();

    // Throttle updates: only update if at least 1 second has passed since last update
    if (!lastUpdate.isValid() || lastUpdate.msecsTo(current) >= 1000) {
        QSettings settings;
        lockscreen h;
        double kcal = calories().value();
        if(kcal < 0)
            kcal = 0;
        bool useMiles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
        h.workoutTrackingUpdate(Speed.value(), Cadence.value(), (uint16_t)m_watt.value(), kcal, StepCount.value(), deviceType(), odometer() * 1000.0, totalCalories().value(), useMiles);

        lastUpdate = current;
    }
    #endif
    #endif
}

void bluetoothdevice::clearStats() {

    elapsed.clear(true);
    moving.clear(true);
    Speed.clear(false);
    KCal.clear(true);
    hrKCal.clear(true);
    activeKCal.clear(true);
    Distance.clear(true);
    Distance1s.clear(true);
    Heart.clear(false);
    m_jouls.clear(true);
    elevationAcc = 0;
    m_watt.clear(false);
    m_rawWatt.clear(false);
    WeightLoss.clear(false);
    WattKg.clear(false);
    Cadence.clear(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].clear(false);
    }
    for(int i=0; i<maxHeatZone(); i++) {
        heatZonesSeconds[i].clear(false);
    }    
}

void bluetoothdevice::setPaused(bool p) {

    paused = p;
    moving.setPaused(p);
    elapsed.setPaused(p);
    Speed.setPaused(p);
    KCal.setPaused(p);
    hrKCal.setPaused(p);
    activeKCal.setPaused(p);
    Distance.setPaused(p);
    Distance1s.setPaused(p);
    Heart.setPaused(p);
    m_jouls.setPaused(p);
    m_watt.setPaused(p);
    m_rawWatt.setPaused(p);
    WeightLoss.setPaused(p);
    WattKg.setPaused(p);
    Cadence.setPaused(p);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setPaused(p);
    }
    for(int i=0; i<maxHeatZone(); i++) {
        heatZonesSeconds[i].setPaused(p);
    }    
}

void bluetoothdevice::setLap() {

    moving.setLap(true);
    elapsed.setLap(true);
    Speed.setLap(false);
    KCal.setLap(true);
    hrKCal.setLap(true);
    activeKCal.setLap(true);
    Distance.setLap(true);
    Distance1s.setLap(true);
    Heart.setLap(false);
    m_jouls.setLap(true);
    m_watt.setLap(false);
    m_rawWatt.setLap(false);
    WeightLoss.setLap(false);
    WattKg.setLap(false);
    Cadence.setLap(false);
    for(int i=0; i<maxHeartZone(); i++) {
        hrZonesSeconds[i].setLap(false);
    }
    for(int i=0; i<maxHeatZone(); i++) {
        heatZonesSeconds[i].setLap(false);
    }    
}

QStringList bluetoothdevice::metrics() {

    QStringList r;
    r.append(QStringLiteral("Speed"));
    r.append(QStringLiteral("Inclination"));
    r.append(QStringLiteral("Cadence"));
    r.append(QStringLiteral("Elevation"));
    r.append(QStringLiteral("Calories"));
    r.append(QStringLiteral("Odometer"));
    r.append(QStringLiteral("Pace"));
    r.append(QStringLiteral("Resistance"));
    r.append(QStringLiteral("Watt"));
    r.append(QStringLiteral("Weight Loss"));
    r.append(QStringLiteral("AVG Watt"));
    r.append(QStringLiteral("FTP"));
    r.append(QStringLiteral("Heart Rate"));
    r.append(QStringLiteral("Fan"));
    r.append(QStringLiteral("Jouls"));
    r.append(QStringLiteral("Lap Elapsed"));
    r.append(QStringLiteral("Elapsed"));
    r.append(QStringLiteral("Moving Time"));
    r.append(QStringLiteral("Peloton Offset"));
    r.append(QStringLiteral("Peloton Resistance"));
    r.append(QStringLiteral("Date Time"));
    r.append(QStringLiteral("Target Resistance"));
    r.append(QStringLiteral("Target Peloton Resistance"));
    r.append(QStringLiteral("Target Cadence"));
    r.append(QStringLiteral("Target Power"));
    r.append(QStringLiteral("Watt/Kg"));
    return r;
}

resistance_t bluetoothdevice::maxResistance() { return 100; }

uint8_t bluetoothdevice::metrics_override_heartrate() {

    QSettings settings;
    QString setting =
        settings.value(QZSettings::peloton_heartrate_metric, QZSettings::default_peloton_heartrate_metric).toString();
    if (!setting.compare(QStringLiteral("Heart Rate"))) {
        return currentHeart().value();
    } else if (!setting.compare(QStringLiteral("Speed"))) {

        return currentSpeed().value();
    } else if (!setting.compare(QStringLiteral("Inclination"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Cadence"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Elevation"))) {

        return elevationGain().value();
    } else if (!setting.compare(QStringLiteral("Calories"))) {

        return calories().value();
    } else if (!setting.compare(QStringLiteral("Odometer"))) {

        return odometer();
    } else if (!setting.compare(QStringLiteral("Pace"))) {

        return currentPace().second();
    } else if (!setting.compare(QStringLiteral("Resistance"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Watt"))) {

        return wattsMetric().value();
    } else if (!setting.compare(QStringLiteral("Weight Loss"))) {

        return weightLoss();
    } else if (!setting.compare(QLatin1String("Watt/Kg"))) {

        return wattKg().value();
    } else if (!setting.compare(QStringLiteral("AVG Watt"))) {

        return wattsMetric().average();
    } else if (!setting.compare(QStringLiteral("FTP"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Fan"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Jouls"))) {

        return jouls().value();
    } else if (!setting.compare(QStringLiteral("Lap Elapsed"))) {

        return lapElapsedTime().second();
    } else if (!setting.compare(QStringLiteral("Elapsed"))) {

        return elapsed.value();
    } else if (!setting.compare(QStringLiteral("Moving Time"))) {

        return movingTime().second();
    } else if (!setting.compare(QStringLiteral("Peloton Offset"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Peloton Resistance"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Date Time"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Target Resistance"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Target Peloton Resistance"))) {

        return 0;
    } else if (!setting.compare(QStringLiteral("Target Power"))) {

        return 0;
    }
    return currentHeart().value();
}

void bluetoothdevice::changeGeoPosition(QGeoCoordinate p, double azimuth, double avgAzimuthNext300Meters) {
    coordinateTS = QDateTime::currentMSecsSinceEpoch();
    coordinateOdometer = odometer();
    coordinate = p;
    this->setAverageAzimuthNext300m(avgAzimuthNext300Meters);
    this->azimuth = azimuth;
}
QGeoCoordinate bluetoothdevice::currentCordinate() {
    if (coordinateTS) {
        double distance = odometer() - coordinateOdometer;
        QGeoCoordinate c = coordinate.atDistanceAndAzimuth(distance * 1000.0, this->azimuth);
        c.setAltitude(coordinate.altitude());
        // qDebug() << "currentCordinate" << c << distance << currentSpeed().value();
        return c;
    }
    return coordinate;
}

void bluetoothdevice::workoutEventStateChanged(bluetoothdevice::WORKOUT_EVENT_STATE state) { lastState = state; }
void bluetoothdevice::setInclination(double inclination) { Inclination = inclination; }
void bluetoothdevice::setGPXFile(QString filename) {
    gpxFileName = filename;
    QFile input(filename);
    if (input.open(QIODevice::ReadOnly)) {
        QByteArray asSaved = input.readAll();
        gpxBase64 = "data:@file/xml;base64," + asSaved.toBase64();
        input.close();
    }
}

void bluetoothdevice::setHeartZone(double hz) {
    HeartZone = hz;
    if(isPaused() == false && currentHeart().value() > 0) {
        hz = hz - 1;
        if(hz >= maxHeartZone() ) {
            hrZonesSeconds[maxHeartZone() - 1].setValue(hrZonesSeconds[maxHeartZone() - 1].value() + 1);
        } else if(hz <= 0) {
            hrZonesSeconds[0].setValue(hrZonesSeconds[0].value() + 1);
        } else {
            hrZonesSeconds[(int)hz].setValue(hrZonesSeconds[(int)hz].value() + 1);
        }
    }
}

void bluetoothdevice::setHeatZone(double heatStrainIndex) {
    // Determine heat zone based on Heat Strain Index values
    uint8_t zone;
    if (heatStrainIndex >= 0 && heatStrainIndex <= 1.99) {
        zone = 1;
    } else if (heatStrainIndex < 3.0) {
        zone = 2;
    } else if (heatStrainIndex < 7.0) {
        zone = 3;
    } else {
        zone = 4;
    }
    
    HeatZone = zone;
    if(isPaused() == false && heatStrainIndex > 0) {
        // Convert to 0-based index for array access
        uint8_t zoneIndex = zone - 1;
        if(zoneIndex < maxHeatZone()) {
            heatZonesSeconds[zoneIndex].setValue(heatZonesSeconds[zoneIndex].value() + 1);
        }
    }
}

uint32_t bluetoothdevice::secondsForHeartZone(uint8_t zone) {
    if(zone < maxHeartZone()) {
       return hrZonesSeconds[zone].value();
    }
    return 0;
}

uint32_t bluetoothdevice::secondsForHeatZone(uint8_t zone) {
    if(zone < maxHeatZone()) {
       return heatZonesSeconds[zone].value();
    }
    return 0;
}
