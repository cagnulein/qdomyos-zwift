#include "devicediscoveryinfo.h"
#include "qzsettings.h"


DeviceDiscoveryInfo::DeviceDiscoveryInfo(bool loadDefaults){
    if(loadDefaults)
        this->loadDefaultValues();
}

void DeviceDiscoveryInfo::setValues(QSettings &settings, bool clear) const {
    if(clear) settings.clear();
    settings.setValue(QZSettings::filter_device, this->filterDevice);
    settings.setValue(QZSettings::applewatch_fakedevice, this->fake_bike);
    settings.setValue(QZSettings::fakedevice_elliptical, this->fakedevice_elliptical);
    settings.setValue(QZSettings::fakedevice_treadmill, this->fakedevice_treadmill);
    settings.setValue(QZSettings::proformtdf4ip, this->proformtdf4ip);
    settings.setValue(QZSettings::proformtreadmillip, this->proformtreadmillip);
    settings.setValue(QZSettings::nordictrack_2950_ip, this->nordictrack_2950_ip);
    settings.setValue(QZSettings::tdf_10_ip, this->tdf_10_ip);
    settings.setValue(QZSettings::cadence_sensor_as_bike, this->csc_as_bike);
    settings.setValue(QZSettings::cadence_sensor_name, this->cscName);
    settings.setValue(QZSettings::power_sensor_as_bike, this->power_as_bike);
    settings.setValue(QZSettings::power_sensor_name, this->powerSensorName);
    settings.setValue(QZSettings::power_sensor_as_treadmill, this->power_as_treadmill);
    settings.setValue(QZSettings::hammer_racer_s, this->hammerRacerS);
    settings.setValue(QZSettings::pafers_treadmill, this->pafers_treadmill);
    settings.setValue(QZSettings::flywheel_life_fitness_ic8, this->flywheel_life_fitness_ic8);
    settings.setValue(QZSettings::toorx_bike, this->toorx_bike);
    settings.setValue(QZSettings::toorx_ftms, this->toorx_ftms);
    settings.setValue(QZSettings::toorx_ftms_treadmill, this->toorx_ftms_treadmill);
    settings.setValue(QZSettings::snode_bike, this->snode_bike);
    settings.setValue(QZSettings::fitplus_bike, this->fitplus_bike);
    settings.setValue(QZSettings::technogym_myrun_treadmill_experimental, this->technogym_myrun_treadmill_experimental);
    settings.setValue(QZSettings::computrainer_serialport, this->computrainer_serial_port);
    settings.setValue(QZSettings::ss2k_peloton, this->ss2k_peloton);
    settings.setValue(QZSettings::ftms_accessory_name, this->ftmsAccessoryName);
    settings.setValue(QZSettings::pafers_treadmill_bh_iboxster_plus, this->pafers_treadmill_bh_iboxster_plus);
    settings.setValue(QZSettings::iconcept_elliptical, this->iconcept_elliptical);
}

void DeviceDiscoveryInfo::getValues(QSettings &settings){
    this->filterDevice = settings.value(QZSettings::filter_device, QZSettings::default_filter_device).toString();
    this->fake_bike = settings.value(QZSettings::applewatch_fakedevice, QZSettings::default_applewatch_fakedevice).toBool();
    this->fakedevice_elliptical = settings.value(QZSettings::fakedevice_elliptical, QZSettings::default_fakedevice_elliptical).toBool();
    this->fakedevice_treadmill = settings.value(QZSettings::fakedevice_treadmill, QZSettings::default_fakedevice_treadmill).toBool();
    this->proformtdf4ip = settings.value(QZSettings::proformtdf4ip, QZSettings::default_proformtdf4ip).toString();
    this->proformtreadmillip = settings.value(QZSettings::proformtreadmillip, QZSettings::default_proformtreadmillip).toString();
    this->nordictrack_2950_ip = settings.value(QZSettings::nordictrack_2950_ip, QZSettings::default_nordictrack_2950_ip).toString();
    this->tdf_10_ip = settings.value(QZSettings::tdf_10_ip, QZSettings::default_tdf_10_ip).toString();
    this->csc_as_bike = settings.value(QZSettings::cadence_sensor_as_bike, QZSettings::default_cadence_sensor_as_bike).toBool();
    this->cscName = settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name).toString();
    this->power_as_bike = settings.value(QZSettings::power_sensor_as_bike, QZSettings::default_power_sensor_as_bike).toBool();
    this->powerSensorName = settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name).toString();
    this->power_as_treadmill = settings.value(QZSettings::power_sensor_as_treadmill, QZSettings::default_power_sensor_as_treadmill).toBool();
    this->hammerRacerS = settings.value(QZSettings::hammer_racer_s, QZSettings::default_hammer_racer_s).toBool();
    this->pafers_treadmill = settings.value(QZSettings::pafers_treadmill, QZSettings::default_pafers_treadmill).toBool();
    this->flywheel_life_fitness_ic8 = settings.value(QZSettings::flywheel_life_fitness_ic8, QZSettings::default_flywheel_life_fitness_ic8).toBool();
    this->toorx_bike = settings.value(QZSettings::toorx_bike, QZSettings::default_toorx_bike).toBool();
    this->toorx_ftms = settings.value(QZSettings::toorx_ftms, QZSettings::default_toorx_ftms).toBool();
    this->toorx_ftms_treadmill = settings.value(QZSettings::toorx_ftms_treadmill, QZSettings::default_toorx_ftms_treadmill).toBool();
    this->snode_bike = settings.value(QZSettings::snode_bike, QZSettings::default_snode_bike).toBool();
    this->fitplus_bike = settings.value(QZSettings::fitplus_bike, QZSettings::default_fitplus_bike).toBool();
    this->technogym_myrun_treadmill_experimental = settings.value(QZSettings::technogym_myrun_treadmill_experimental, QZSettings::default_technogym_myrun_treadmill_experimental).toBool();
    this->computrainer_serial_port = settings.value(QZSettings::computrainer_serialport, QZSettings::default_computrainer_serialport).toString();
    this->ss2k_peloton = settings.value(QZSettings::ss2k_peloton, QZSettings::default_ss2k_peloton).toBool();
    this->ftmsAccessoryName = settings.value(QZSettings::ftms_accessory_name, QZSettings::default_ftms_accessory_name).toString();
    this->pafers_treadmill_bh_iboxster_plus = settings.value(QZSettings::pafers_treadmill_bh_iboxster_plus, QZSettings::default_pafers_treadmill_bh_iboxster_plus).toBool();
    this->iconcept_elliptical = settings.value(QZSettings::iconcept_elliptical, QZSettings::default_iconcept_elliptical).toBool();
}

void DeviceDiscoveryInfo::loadDefaultValues() {
    // generate some settings with some random ids
    QSettings settings("21f0b218-895f-11ed-a1eb-0242ac120002", "21f0b218-895f-11ed-a1eb-0242ac120002");

    // clear the settings in case they exist from some other call
    settings.clear();

    this->getValues(settings);

    // clear the settings
    settings.clear();
}

