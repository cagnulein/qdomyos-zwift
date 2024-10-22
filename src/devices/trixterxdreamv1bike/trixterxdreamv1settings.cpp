#include "trixterxdreamv1settings.h"

#include "qzsettings.h"

const QString trixterxdreamv1settings::keys::Enabled = QZSettings::trixter_xdream_v1_bike_enabled;
const QString trixterxdreamv1settings::keys::HeartRateEnabled = QZSettings::trixter_xdream_v1_bike_heartrate_enabled;
const QString trixterxdreamv1settings::keys::SteeringEnabled = QZSettings::trixter_xdream_v1_bike_steering_enabled;
const QString trixterxdreamv1settings::keys::SteeringCalibrationLeft = QZSettings::trixter_xdream_v1_bike_steering_l;
const QString trixterxdreamv1settings::keys::SteeringCalibrationCenterLeft =QZSettings::trixter_xdream_v1_bike_steering_cl;
const QString trixterxdreamv1settings::keys::SteeringCalibrationCenterRight = QZSettings::trixter_xdream_v1_bike_steering_cr;
const QString trixterxdreamv1settings::keys::SteeringCalibrationRight = QZSettings::trixter_xdream_v1_bike_steering_r;
const QString trixterxdreamv1settings::keys::SteeringCalibrationMAX = QZSettings::trixter_xdream_v1_bike_steering_max;
const QString trixterxdreamv1settings::keys::ConnectionTimeoutMilliseconds = QZSettings::trixter_xdream_v1_bike_connection_timeout_ms;


template <typename T>
T trixterxdreamv1settings::updateField(T& member, const T newValue) {
    QMutexLocker locker(&this->mutex);
    if(member!=newValue) {
        member = newValue;
        this->version++;
    }
    return newValue;
}


uint32_t trixterxdreamv1settings::get_version() {
    QMutexLocker locker(&this->mutex);
    return this->version;
}

bool trixterxdreamv1settings::get_enabled() {
    QMutexLocker locker(&this->mutex);
    return this->enabled;
}

bool trixterxdreamv1settings::set_enabled(bool value) {
    return this->updateField(this->enabled, value);
}

bool trixterxdreamv1settings::get_heartRateEnabled(){
    QMutexLocker locker(&this->mutex);
    return this->heartRateEnabled;
}

bool trixterxdreamv1settings::set_heartRateEnabled(bool value) {
    return this->updateField(this->heartRateEnabled, value);
}

bool trixterxdreamv1settings::get_steeringEnabled()  {
    QMutexLocker locker(&this->mutex);
    return this->steeringEnabled;
}

bool trixterxdreamv1settings::set_steeringEnabled(bool value) {
    return this->updateField(this->steeringEnabled, value);
}

trixterxdreamv1settings::steeringCalibrationInfo trixterxdreamv1settings::get_steeringCalibration() {
    QMutexLocker locker(&this->mutex);
    return this->steeringCalibration;
}

void trixterxdreamv1settings::set_steeringCalibration(const trixterxdreamv1settings::steeringCalibrationInfo value) {
    if(!value.isValid())
        throw "Invalid argument.";

    QMutexLocker locker(&this->mutex);
    if(!(this->steeringCalibration==value)) {
        this->steeringCalibration = value;
        this->version++;
    }
}

uint16_t trixterxdreamv1settings::get_connectionTimeoutMilliseconds() {
    QMutexLocker locker(&this->mutex);
    return this->DefaultConnectionTimeoutMilliseconds;
}

void trixterxdreamv1settings::set_connectionTimeoutMilliseconds(uint16_t value) {
    value = this->clip(MinConnectionTimeoutMilliseconds, MaxConnectionTimeoutMilliseconds, value);
    this->updateField(this->connectionTimeoutMilliseconds, value);
}

trixterxdreamv1settings::trixterxdreamv1settings() {
    QSettings defaultSettings;
    this->Load(defaultSettings);
    this->version = 1;
}

trixterxdreamv1settings::trixterxdreamv1settings(const QSettings &settings) {
    this->Load(settings);
    this->version = 1;
}

void trixterxdreamv1settings::Load() {
    QSettings settings;
    this->Load(settings);
}

void trixterxdreamv1settings::Load(const QSettings &settings) {
    QMutexLocker locker(&this->mutex);
    this->set_enabled(settings.value(keys::Enabled, DefaultEnabled).toBool());
    this->set_heartRateEnabled(settings.value(keys::HeartRateEnabled, DefaultHeartRateEnabled).toBool());
    this->set_steeringEnabled(settings.value(keys::SteeringEnabled, DefaultSteeringEnabled).toBool());
    this->set_connectionTimeoutMilliseconds(settings.value(keys::ConnectionTimeoutMilliseconds, DefaultConnectionTimeoutMilliseconds).toUInt());

    int32_t l = settings.value(keys::SteeringCalibrationLeft, DefaultSteeringCalibrationL).toInt();
    int32_t lc = settings.value(keys::SteeringCalibrationCenterLeft, DefaultSteeringCalibrationCL).toInt();
    int32_t lr = settings.value(keys::SteeringCalibrationCenterRight, DefaultSteeringCalibrationCR).toInt();
    int32_t r = settings.value(keys::SteeringCalibrationRight, DefaultSteeringCalibrationR).toInt();

    int32_t xx  = settings.value(keys::SteeringCalibrationCenterLeft, 1).toInt();

    steeringCalibrationInfo sc(l,lc,lr, r);
    this->set_steeringCalibration(sc);
}

/*
void trixterxdreamv1bikesettings::Save() {
    QSettings settings;
    this->Save(settings);
}

void trixterxdreamv1bikesettings::Save(const QSettings &settings) {
    QMutexLocker locker(&this->mutex);
    settings.value(keys::Enabled).setValue(this->enabled);
    settings.value(keys::HeartRateEnabled).setValue(this->heartRateEnabled);
    settings.value(keys::SteeringEnabled).setValue(this->steeringEnabled);

}

*/
