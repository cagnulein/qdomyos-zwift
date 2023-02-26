#include "horizongr7biketestdata.h" 
#include "horizongr7bike.h"

HorizonGR7BikeTestData::HorizonGR7BikeTestData() : BikeTestData("Horizon GR7 Bike") {
    this->addDeviceName("JFIC", comparison::StartsWithIgnoreCase);
}

deviceType HorizonGR7BikeTestData::get_expectedDeviceType() const { return deviceType::HorizonGr7Bike; }

bool HorizonGR7BikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<horizongr7bike*>(detectedDevice)!=nullptr;
}
