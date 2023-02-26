#include "nautilustreadmilltestdata.h" 
#include "nautilustreadmill.h"

NautilusTreadmillTestData::NautilusTreadmillTestData() : TreadmillTestData("Nautilus Treadmill") {
    this->addDeviceName("NAUTILUS T", comparison::StartsWithIgnoreCase);
}

deviceType NautilusTreadmillTestData::get_expectedDeviceType() const { return deviceType::NautilusTreadmill; }

bool NautilusTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<nautilustreadmill*>(detectedDevice)!=nullptr;
}
