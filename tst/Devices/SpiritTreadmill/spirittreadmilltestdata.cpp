#include "spirittreadmilltestdata.h" 
#include "spirittreadmill.h"

SpiritTreadmillTestData::SpiritTreadmillTestData(): TreadmillTestData("Spirit Treadmill") {

    this->addDeviceName("XT385", comparison::StartsWithIgnoreCase);
    this->addDeviceName("XT485", comparison::StartsWithIgnoreCase);
    this->addDeviceName("XT800", comparison::StartsWithIgnoreCase);
    this->addDeviceName("XT900", comparison::StartsWithIgnoreCase);
}

deviceType SpiritTreadmillTestData::get_expectedDeviceType() const { return deviceType::SpiritTreadmill; }

bool SpiritTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<spirittreadmill*>(detectedDevice)!=nullptr;
}
