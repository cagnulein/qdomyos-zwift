#include "trixterxdreamv1settings.h"

const QString trixterxdreamv1settings::keys::Enabled = QStringLiteral("trixter_xdream_v1_bike");
const QString trixterxdreamv1settings::keys::HeartRateEnabled = QStringLiteral("trixter_xdream_v1_bike_heartrate_enabled");
const QString trixterxdreamv1settings::keys::SteeringEnabled = QStringLiteral("trixter_xdream_v1_bike_steering_enabled");
const QString trixterxdreamv1settings::keys::SteeringCenterOffset = QStringLiteral("trixter_xdream_v1_bike_steering_center_offset");
const QString trixterxdreamv1settings::keys::SteeringDeadZoneWidth =QStringLiteral("trixter_xdream_v1_bike_steering_deadzone_width");
const QString trixterxdreamv1settings::keys::SteeringSensitivityLeft = QStringLiteral("trixter_xdream_v1_bike_steering_sensitivity_left");
const QString trixterxdreamv1settings::keys::SteeringSensitivityRight= QStringLiteral("trixter_xdream_v1_bike_steering_sensitivity_right");


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

int8_t trixterxdreamv1settings::get_steeringCenterOffsetPercentage() {
    QMutexLocker locker(&this->mutex);
    return this->steeringCenterOffsetPercentage;
}

int8_t trixterxdreamv1settings::set_steeringCenterOffsetPercentage(int8_t value) {
    auto newValue = clip((int8_t)-MaxSteeringCenterOffsetPercentage, MaxSteeringCenterOffsetPercentage, value);
    return this->updateField(this->steeringCenterOffsetPercentage, newValue);
}

uint8_t trixterxdreamv1settings::get_steeringDeadZoneWidthPercentage() {
    QMutexLocker locker(&this->mutex);
    return this->steeringDeadZoneWidthPercentage;
}

uint8_t trixterxdreamv1settings::set_steeringDeadZoneWidthPercentage(uint8_t value) {
    auto newValue = clip(MinSteeringDeadZoneWidthPercentage, MaxSteeringDeadZoneWidthPercentage, value);
    return this->updateField(this->steeringDeadZoneWidthPercentage, newValue);
}

uint8_t trixterxdreamv1settings::get_steeringSensitivityLeft()  {
    QMutexLocker locker(&this->mutex);
    return this->steeringSensitivityLeft;
}

uint8_t trixterxdreamv1settings::set_steeringSensitivityLeft(uint8_t value) {
    auto newValue = clip(MinSteeringSensitivityPercentage, MaxSteeringSensitivityPercentage, value);
    return this->updateField(this->steeringSensitivityLeft, newValue);
}

uint8_t trixterxdreamv1settings::get_steeringSensitivityRight()  {
    QMutexLocker locker(&this->mutex);
    return this->steeringSensitivityRight;
}

uint8_t trixterxdreamv1settings::set_steeringSensitivityRight(uint8_t value) {
    auto newValue = clip(MinSteeringSensitivityPercentage, MaxSteeringSensitivityPercentage, value);
    return this->updateField(this->steeringSensitivityRight, newValue);
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
    this->set_steeringCenterOffsetPercentage(settings.value(keys::SteeringCenterOffset, DefaultSteeringCenterOffsetPercentage).toUInt());
    this->set_steeringDeadZoneWidthPercentage(settings.value(keys::SteeringDeadZoneWidth, DefaultSteeringDeadZoneWidthPercentage).toUInt());
    this->set_steeringSensitivityLeft(settings.value(keys::SteeringSensitivityLeft, DefaultSteeringSensitivity).toUInt());
    this->set_steeringSensitivityRight(settings.value(keys::SteeringSensitivityRight, DefaultSteeringSensitivity).toUInt());
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
    settings.value(keys::SteeringCenter).setValue(this->steeringCenter);
    settings.value(keys::SteeringDeadZoneWidth).setValue(this->steeringDeadZoneWidth);
    settings.value(keys::SteeringSensitivityLeft).setValue(this->steeringSensitivityLeft);
    settings.value(keys::SteeringSensitivityRight).setValue(this->steeringSensitivityRight);
}

*/
