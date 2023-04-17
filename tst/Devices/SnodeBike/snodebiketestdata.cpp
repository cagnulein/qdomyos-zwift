#include "snodebiketestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"
#include "snodebike.h"

SnodeBikeTestData::SnodeBikeTestData(std::string testName) : BikeTestData(testName) { }

void SnodeBikeTestData::configureExclusions() {
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new FTMSBike1TestData());
    this->exclude(new FTMSBike2TestData());
}

deviceType SnodeBikeTestData::get_expectedDeviceType() const { return deviceType::SnodeBike; }

bool SnodeBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<snodebike*>(detectedDevice)!=nullptr;
}

bool SnodeBike1TestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.snode_bike = enable;
    return true;
}

SnodeBike1TestData::SnodeBike1TestData() : SnodeBikeTestData("Snode Bike") {
    this->addDeviceName("FS-", comparison::StartsWith);
}

void SnodeBike2TestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if (enable) {
        config.horizon_treadmill_force_ftms = false;
        configurations.push_back(config);
    } else {
        // Basic case where the device is disabled in the settings
        config.horizon_treadmill_force_ftms = true;
        configurations.push_back(config);
    }
}

SnodeBike2TestData::SnodeBike2TestData() : SnodeBikeTestData("Snode Bike TF") {
    this->addDeviceName("TF-", comparison::StartsWithIgnoreCase);
}
