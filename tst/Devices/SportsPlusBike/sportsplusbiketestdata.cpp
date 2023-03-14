#include "sportsplusbiketestdata.h"
#include "sportsplusbike.h"

SportsPlusBikeTestData::SportsPlusBikeTestData() : BikeTestData("Sports Plus Bike") {
    this->addDeviceName("CARDIOFIT", comparison::StartsWithIgnoreCase);
}

deviceType SportsPlusBikeTestData::get_expectedDeviceType() const { return deviceType::SportsPlusBike; }

bool SportsPlusBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<sportsplusbike*>(detectedDevice)!=nullptr;
}
