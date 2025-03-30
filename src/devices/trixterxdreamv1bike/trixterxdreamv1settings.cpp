#include "trixterxdreamv1settings.h"

#include "qzsettings.h"

#define TrixterSettingsKeys_Enabled QZSettings::trixter_xdream_v1_bike_enabled
#define TrixterSettingsKeys_HeartRateEnabled QZSettings::trixter_xdream_v1_bike_heartrate_enabled
#define TrixterSettingsKeys_SteeringEnabled QZSettings::trixter_xdream_v1_bike_steering_enabled
#define TrixterSettingsKeys_SteeringCalibrationLeft QZSettings::trixter_xdream_v1_bike_steering_l
#define TrixterSettingsKeys_SteeringCalibrationCenterLeft QZSettings::trixter_xdream_v1_bike_steering_cl
#define TrixterSettingsKeys_SteeringCalibrationCenterRight QZSettings::trixter_xdream_v1_bike_steering_cr
#define TrixterSettingsKeys_SteeringCalibrationRight QZSettings::trixter_xdream_v1_bike_steering_r
#define TrixterSettingsKeys_SteeringCalibrationMAX QZSettings::trixter_xdream_v1_bike_steering_max
#define TrixterSettingsKeys_ConnectionTimeoutMilliseconds QZSettings::trixter_xdream_v1_bike_connection_timeout_ms


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
    this->set_enabled(settings.value(TrixterSettingsKeys_Enabled, DefaultEnabled).toBool());
    this->set_heartRateEnabled(settings.value(TrixterSettingsKeys_HeartRateEnabled, DefaultHeartRateEnabled).toBool());
    this->set_steeringEnabled(settings.value(TrixterSettingsKeys_SteeringEnabled, DefaultSteeringEnabled).toBool());
    this->set_connectionTimeoutMilliseconds(settings.value(TrixterSettingsKeys_ConnectionTimeoutMilliseconds, DefaultConnectionTimeoutMilliseconds).toUInt());

    int32_t l = settings.value(TrixterSettingsKeys_SteeringCalibrationLeft, DefaultSteeringCalibrationL).toInt();
    int32_t lc = settings.value(TrixterSettingsKeys_SteeringCalibrationCenterLeft, DefaultSteeringCalibrationCL).toInt();
    int32_t lr = settings.value(TrixterSettingsKeys_SteeringCalibrationCenterRight, DefaultSteeringCalibrationCR).toInt();
    int32_t r = settings.value(TrixterSettingsKeys_SteeringCalibrationRight, DefaultSteeringCalibrationR).toInt();

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
