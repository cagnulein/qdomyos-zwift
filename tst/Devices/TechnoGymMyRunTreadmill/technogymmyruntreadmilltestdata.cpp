#include "technogymmyruntreadmilltestdata.h" 
#include "technogymmyruntreadmill.h"

bool TechnoGymMyRunTreadmillTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.technogym_myrun_treadmill_experimental = !enable;
    return true;
}

TechnoGymMyRunTreadmillTestData::TechnoGymMyRunTreadmillTestData() : TreadmillTestData("Technogym MyRun Treadmill") {
    this->addDeviceName("MYRUN ", comparison::StartsWithIgnoreCase);
    this->addDeviceName("MERACH-U3", comparison::StartsWithIgnoreCase);
}

deviceType TechnoGymMyRunTreadmillTestData::get_expectedDeviceType() const { return deviceType::TechnoGymMyRunTreadmill; }

bool TechnoGymMyRunTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<technogymmyruntreadmill*>(detectedDevice)!=nullptr;
}
