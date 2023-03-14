#include "echelonrowertestdata.h" 
#include "echelonrower.h"

EchelonRowerTestData::EchelonRowerTestData() : RowerTestData("Echelon Rower") {
    this->addDeviceName("ECH-ROW", comparison::StartsWith);
    this->addDeviceName("ROWSPORT-", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ROW-S", comparison::StartsWith);
}


void EchelonRowerTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto virtualDevice = QZLockscreenFunctions::configurationType::BIKE;

    for(int i=0; i<8; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        config.virtual_device_rower = i&4;
        bool pelotonActive = config.ios_peloton_workaround && config.bike_cadence_sensor && !config.virtual_device_rower;
        configurations.push_back(LockscreenFunctionsTestData(virtualDevice, pelotonActive, false, config));
    }
}

deviceType EchelonRowerTestData::get_expectedDeviceType() const { return deviceType::EchelonRower; }

bool EchelonRowerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<echelonrower*>(detectedDevice)!=nullptr;
}
