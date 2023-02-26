#include "skandikawirybiketestdata.h"
#include "skandikawiribike.h"

SkandikaWiryBikeTestData::SkandikaWiryBikeTestData() : BikeTestData("Skandika Wiry Bike") {
    this->addDeviceName("BFCP", comparison::StartsWithIgnoreCase);
}

deviceType SkandikaWiryBikeTestData::get_expectedDeviceType() const { return deviceType::SkandikawiriBike; }

bool SkandikaWiryBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<skandikawiribike*>(detectedDevice)!=nullptr;
}
