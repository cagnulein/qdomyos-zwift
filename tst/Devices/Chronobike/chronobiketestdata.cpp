#include "chronobiketestdata.h" 
#include "chronobike.h"

ChronobikeTestData::ChronobikeTestData() : BikeTestData("Chronobike") {
    this->addDeviceName("CHRONO ", comparison::StartsWithIgnoreCase);
}

deviceType ChronobikeTestData::get_expectedDeviceType() const { return deviceType::ChronoBike; }

bool ChronobikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<chronobike*>(detectedDevice)!=nullptr;
}
