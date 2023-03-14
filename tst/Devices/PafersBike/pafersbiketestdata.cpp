#include "pafersbiketestdata.h" 
#include "pafersbike.h"

bool PafersBikeTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    // the treadmill is given priority
    info.pafers_treadmill = !enable;
    return true;
}

PafersBikeTestData::PafersBikeTestData() : BikeTestData("Pafers Bike") {
    this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
}

deviceType PafersBikeTestData::get_expectedDeviceType() const { return deviceType::PafersBike; }

bool PafersBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<pafersbike*>(detectedDevice)!=nullptr;
}
