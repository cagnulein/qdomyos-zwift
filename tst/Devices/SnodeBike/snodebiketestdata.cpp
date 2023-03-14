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

SnodeBike2TestData::SnodeBike2TestData() : SnodeBikeTestData("Snode Bike TF") {
    this->addDeviceName("TF-", comparison::StartsWith);
}
