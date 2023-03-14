#include "mepanelbiketestdata.h" 
#include "mepanelbike.h"

MepanelBikeTestData::MepanelBikeTestData() : BikeTestData("Mepanel Bike") {
    this->addDeviceName("MEPANEL", comparison::StartsWithIgnoreCase);
}

deviceType MepanelBikeTestData::get_expectedDeviceType() const { return deviceType::MepanelBike; }

bool MepanelBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<mepanelbike*>(detectedDevice)!=nullptr;
}
