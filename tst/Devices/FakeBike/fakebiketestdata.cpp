#include "fakebiketestdata.h" 
#include "fakebike.h"

bool FakeBikeTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.fake_bike = enable;
    return true;
}

FakeBikeTestData::FakeBikeTestData() : BikeTestData("Fake Bike"){
    this->addDeviceName("", comparison::StartsWithIgnoreCase);
}

deviceType FakeBikeTestData::get_expectedDeviceType() const { return deviceType::FakeBike; }

bool FakeBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<fakebike*>(detectedDevice)!=nullptr;
}
