#include "solebiketestdata.h" 
#include "solebike.h"

SoleBikeTestData::SoleBikeTestData() : BikeTestData("Sole Bike") {
    this->addDeviceName("LCB", comparison::StartsWithIgnoreCase);
    this->addDeviceName("R92", comparison::StartsWithIgnoreCase);
}

deviceType SoleBikeTestData::get_expectedDeviceType() const { return deviceType::SoleBike; }

bool SoleBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<solebike*>(detectedDevice)!=nullptr;
}
