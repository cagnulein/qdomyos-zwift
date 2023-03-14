#include "fitplusbiketestdata.h"

#include "Devices/FTMSBike/ftmsbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"
#include "fitplusbike.h"


FitPlusBikeMRKTestData::FitPlusBikeMRKTestData() : BikeTestData("FitPlus Bike (MRK, no settings)"){

    this->addDeviceName("MRK-", comparison::StartsWith);
}

void FitPlusBikeMRKTestData::configureExclusions() {
    this->exclude(new FTMSBike1TestData());
    this->exclude(new FTMSBike2TestData());
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
}

deviceType FitPlusBikeMRKTestData::get_expectedDeviceType() const { return deviceType::FitPlusBike; }

bool FitPlusBikeMRKTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
}

bool FitPlusBikeFSTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.fitplus_bike = enable;
    return true;
}

FitPlusBikeFSTestData::FitPlusBikeFSTestData() : BikeTestData("FitPlus Bike"){
    this->addDeviceName("FS-", comparison::StartsWith);
}

deviceType FitPlusBikeFSTestData::get_expectedDeviceType() const { return deviceType::FitPlusBike; }

bool FitPlusBikeFSTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
}
