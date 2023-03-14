#include "shuaa5treadmilltestdata.h" 
#include "shuaa5treadmill.h"

Shuaa5TreadmillTestData::Shuaa5TreadmillTestData() : TreadmillTestData("Shuaa5 Treadmill") {
    this->addDeviceName("ZW-", comparison::StartsWithIgnoreCase);
}

deviceType Shuaa5TreadmillTestData::get_expectedDeviceType() const { return deviceType::Shuaa5Treadmill; }

bool Shuaa5TreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<shuaa5treadmill*>(detectedDevice)!=nullptr;
}
