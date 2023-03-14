#include "technogymmyruntreadmillrfcommtestdata.h" 
#include "technogymmyruntreadmillrfcomm.h"

bool TechnogymMyRunTreadmillRFCommTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.technogym_myrun_treadmill_experimental = enable;
    return true;
}

TechnogymMyRunTreadmillRFCommTestData::TechnogymMyRunTreadmillRFCommTestData() : TreadmillTestData("Technogym MyRun Treadmill RF Comm") {
    this->addDeviceName("MYRUN ", comparison::StartsWithIgnoreCase);
    this->addDeviceName("MERACH-U3", comparison::StartsWithIgnoreCase);
}

deviceType TechnogymMyRunTreadmillRFCommTestData::get_expectedDeviceType() const { return deviceType::TechnoGymMyRunTreadmillRfComm; }

bool TechnogymMyRunTreadmillRFCommTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<technogymmyruntreadmillrfcomm*>(detectedDevice)!=nullptr;
}
