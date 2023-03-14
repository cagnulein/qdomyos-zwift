#include "solef80treadmilltestdata.h" 
#include "solef80treadmill.h"

SoleF80TreadmillTestData::SoleF80TreadmillTestData() : TreadmillTestData("Sole F80") {
    this->addDeviceName("F80", comparison::StartsWithIgnoreCase);
    this->addDeviceName("F65", comparison::StartsWithIgnoreCase);
    this->addDeviceName("S77", comparison::StartsWithIgnoreCase);
    this->addDeviceName("TT8", comparison::StartsWithIgnoreCase);
    this->addDeviceName("F63", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ST90", comparison::StartsWithIgnoreCase);
    this->addDeviceName("F85", comparison::StartsWithIgnoreCase);
}

deviceType SoleF80TreadmillTestData::get_expectedDeviceType() const { return deviceType::SoleF80Treadmill; }

bool SoleF80TreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<solef80treadmill*>(detectedDevice)!=nullptr;
}
