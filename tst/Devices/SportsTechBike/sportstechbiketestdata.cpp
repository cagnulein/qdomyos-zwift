#include "sportstechbiketestdata.h" 
#include "sportstechbike.h"

SportsTechBikeTestData::SportsTechBikeTestData() : BikeTestData("Sport Tech Bike") {
    this->addDeviceName("EW-BK", comparison::StartsWithIgnoreCase);
}

deviceType SportsTechBikeTestData::get_expectedDeviceType() const { return deviceType::SportsTechBike; }

bool SportsTechBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<sportstechbike*>(detectedDevice)!=nullptr;
}
