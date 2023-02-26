#include "mcfbiketestdata.h" 
#include "mcfbike.h"

MCFBikeTestData::MCFBikeTestData() : BikeTestData("MCF Bike") {
    this->addDeviceName("MCF-", comparison::StartsWithIgnoreCase);
}

deviceType MCFBikeTestData::get_expectedDeviceType() const { return deviceType::MCFBike; }

bool MCFBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<mcfbike*>(detectedDevice)!=nullptr;
}
