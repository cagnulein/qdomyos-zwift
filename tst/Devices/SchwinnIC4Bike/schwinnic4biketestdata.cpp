#include "schwinnic4biketestdata.h" 
#include "schwinnic4bike.h"

SchwinnIC4BikeTestData::SchwinnIC4BikeTestData() : BikeTestData("Schwinn IC4 Bike") {

    this->addDeviceName("IC BIKE", comparison::StartsWithIgnoreCase);
    this->addDeviceName("C7-", comparison::StartsWithIgnoreCase);
    this->addDeviceName("C9/C10", comparison::StartsWithIgnoreCase);

    // 17 characters, beginning with C7-
    this->addInvalidDeviceName("C7-45678901234567", comparison::IgnoreCase);
}

deviceType SchwinnIC4BikeTestData::get_expectedDeviceType() const { return deviceType::SchwinnIC4Bike; }

bool SchwinnIC4BikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<schwinnic4bike*>(detectedDevice)!=nullptr;
}
