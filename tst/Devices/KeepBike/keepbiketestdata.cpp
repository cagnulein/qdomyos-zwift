#include "keepbiketestdata.h" 
#include "keepbike.h"

KeepBikeTestData::KeepBikeTestData() : BikeTestData("Keep Bike") {
    this->addDeviceName("KEEP_BIKE_", comparison::StartsWithIgnoreCase);
}

deviceType KeepBikeTestData::get_expectedDeviceType() const { return deviceType::KeepBike; }

bool KeepBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<keepbike*>(detectedDevice)!=nullptr;
}
