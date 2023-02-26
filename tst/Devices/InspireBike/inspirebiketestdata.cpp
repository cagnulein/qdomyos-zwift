#include "inspirebiketestdata.h" 
#include "inspirebike.h"

InspireBikeTestData::InspireBikeTestData() : BikeTestData("Inspire Bike") {
    this->addDeviceName("IC", comparison::StartsWithIgnoreCase, 8);
}

deviceType InspireBikeTestData::get_expectedDeviceType() const { return deviceType::InspireBike; }

bool InspireBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<inspirebike*>(detectedDevice)!=nullptr;
}
