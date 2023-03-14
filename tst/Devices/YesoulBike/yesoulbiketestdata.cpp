#include "yesoulbiketestdata.h" 
#include "yesoulbike.h"

YesoulBikeTestData::YesoulBikeTestData() : BikeTestData("Yesoul Bike") {
    this->addDeviceName(yesoulbike::bluetoothName, comparison::StartsWith);
}

deviceType YesoulBikeTestData::get_expectedDeviceType() const { return deviceType::YesoulBike; }

bool YesoulBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<yesoulbike*>(detectedDevice)!=nullptr;
}
