#include "concept2skiergtestdata.h" 
#include "concept2skierg.h"

void Concept2SkiErgTestData::configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const {
    DeviceDiscoveryInfo config(info);
    auto bike = QZLockscreenFunctions::configurationType::BIKE;

    // Enabling
    config.ios_peloton_workaround = true;
    config.bike_cadence_sensor = true;
    configurations.push_back(LockscreenFunctionsTestData(bike, true, false,config));

    // Disabling
    for(int i=0; i<3; i++) {
        config.ios_peloton_workaround = i&1;
        config.bike_cadence_sensor = i&2;
        configurations.push_back(LockscreenFunctionsTestData(bike, false, false, config));
    }
}

Concept2SkiErgTestData::Concept2SkiErgTestData() : RowerTestData("Concept2 Ski Erg") {
    this->addDeviceName("PM5", "SKI", comparison::IgnoreCase);
}

deviceType Concept2SkiErgTestData::get_expectedDeviceType() const { return deviceType::Concept2SkiErg; }

bool Concept2SkiErgTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<concept2skierg*>(detectedDevice)!=nullptr;
}
