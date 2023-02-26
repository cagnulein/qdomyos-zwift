#include "nautilusbiketestdata.h" 
#include "nautilusbike.h"

NautilusBikeTestData::NautilusBikeTestData(): BikeTestData("Nautilus Bike") {
    this->addDeviceName("NAUTILUS B", comparison::StartsWithIgnoreCase);
}

deviceType NautilusBikeTestData::get_expectedDeviceType() const { return deviceType::NautilusBike; }

bool NautilusBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<nautilusbike*>(detectedDevice)!=nullptr;
}
