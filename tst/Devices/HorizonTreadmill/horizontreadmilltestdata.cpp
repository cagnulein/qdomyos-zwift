#include "horizontreadmilltestdata.h" 
#include "horizontreadmill.h"

HorizonTreadmillTestData::HorizonTreadmillTestData() : TreadmillTestData("Horizon Treadmill") {

    this->addDeviceName("HORIZON", comparison::StartsWithIgnoreCase);
    this->addDeviceName("AFG SPORT", comparison::StartsWithIgnoreCase);
    this->addDeviceName("WLT2541", comparison::StartsWithIgnoreCase);

    // FTMS
    this->addDeviceName("T318_", comparison::StartsWithIgnoreCase);
    this->addDeviceName("T218_", comparison::StartsWithIgnoreCase);
    this->addDeviceName("TRX3500", comparison::StartsWithIgnoreCase);
    this->addDeviceName("JFTMPARAGON", comparison::StartsWithIgnoreCase);
	this->addDeviceName("PARAGON X", comparison::StartsWithIgnoreCase); 
    this->addDeviceName("JFTM", comparison::StartsWithIgnoreCase);
    this->addDeviceName("CT800", comparison::StartsWithIgnoreCase);
    this->addDeviceName("TRX4500", comparison::StartsWithIgnoreCase);
    this->addDeviceName("MOBVOI TM", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ESANGLINKER", comparison::StartsWithIgnoreCase);
    this->addDeviceName("DK202000725", comparison::StartsWithIgnoreCase);
    this->addDeviceName("MX-TM ", comparison::StartsWithIgnoreCase);
	this->addDeviceName("MATRIXTF50", comparison::StartsWithIgnoreCase);
}

deviceType HorizonTreadmillTestData::get_expectedDeviceType() const { return deviceType::HorizonTreadmill; }

bool HorizonTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
}

void HorizonTreadmillToorxTestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if (enable) {
        config.toorx_ftms_treadmill = true;
        config.toorx_ftms = false;
        configurations.push_back(config);
    } else {
        // Basic case where the device is disabled in the settings
        config.toorx_ftms_treadmill = false;
        config.toorx_ftms = false;
        configurations.push_back(config);

        // Basic case where the device is disabled in the settings and has an excluding settings
        config.toorx_ftms_treadmill = false;
        config.toorx_ftms = true;
        configurations.push_back(config);

        // Enabled in settings, but with excluding setting
        config.toorx_ftms_treadmill = true;
        config.toorx_ftms = true;
        configurations.push_back(config);
    }
}

HorizonTreadmillToorxTestData::HorizonTreadmillToorxTestData() : TreadmillTestData("Horizon Treadmill (Toorx)") {
    this->addDeviceName("TOORX", comparison::StartsWithIgnoreCase);
    this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase);
}

deviceType HorizonTreadmillToorxTestData::get_expectedDeviceType() const { return deviceType::HorizonTreadmill; }

bool HorizonTreadmillToorxTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
}

void HorizonTreadmillBodyToneTestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if (enable) {
        config.horizon_treadmill_force_ftms = true;
        configurations.push_back(config);
    } else {
        // Basic case where the device is disabled in the settings
        config.horizon_treadmill_force_ftms = false;
        configurations.push_back(config);
    }
}

HorizonTreadmillBodyToneTestData::HorizonTreadmillBodyToneTestData() : TreadmillTestData("Horizon Treadmill (Bodytone)") {
    this->addDeviceName("TF-", comparison::StartsWithIgnoreCase);
}

deviceType HorizonTreadmillBodyToneTestData::get_expectedDeviceType() const { return deviceType::HorizonTreadmill; }

bool HorizonTreadmillBodyToneTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
}
