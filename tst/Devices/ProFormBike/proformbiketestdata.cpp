#include "proformbiketestdata.h" 
#include "proformbike.h"

ProFormBikeTestData::ProFormBikeTestData() : BikeTestData("ProForm Bike") {
    this->addDeviceName("I_EB", comparison::StartsWith);
    this->addDeviceName("I_SB", comparison::StartsWith);
}

deviceType ProFormBikeTestData::get_expectedDeviceType() const { return deviceType::ProformBike; }

bool ProFormBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<proformbike*>(detectedDevice)!=nullptr;
}
