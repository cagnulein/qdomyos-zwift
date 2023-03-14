#include "smartrowrowertestdata.h" 
#include "smartrowrower.h"

SmartRowRowerTestData::SmartRowRowerTestData() : RowerTestData("Smart Row Rower") {
    this->addDeviceName("SMARTROW", comparison::StartsWith);
}

void SmartRowRowerTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto virtualDevice = QZLockscreenFunctions::configurationType::BIKE;

    for(int i=0; i<8; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        config.virtual_device_rower = i&4;
        bool pelotonEnabled = config.ios_peloton_workaround && config.bike_cadence_sensor; // virtual_device_rower not consdered by this rower
        configurations.push_back(LockscreenFunctionsTestData(virtualDevice, pelotonEnabled, false, config));
    }
}

deviceType SmartRowRowerTestData::get_expectedDeviceType() const { return deviceType::SmartRowRower; }

bool SmartRowRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<smartrowrower*>(detectedDevice)!=nullptr;
}
