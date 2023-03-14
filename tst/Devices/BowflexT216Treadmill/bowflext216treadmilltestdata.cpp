#include "bowflext216treadmilltestdata.h" 
#include "bowflext216treadmill.h"

BowflexT216TreadmillTestData::BowflexT216TreadmillTestData() : TreadmillTestData("Bowflex T216 Treadmill") {
    this->addDeviceName("BOWFLEX T", comparison::StartsWithIgnoreCase);
}

deviceType BowflexT216TreadmillTestData::get_expectedDeviceType() const { return deviceType::BowflexT216Treadmill; }

bool BowflexT216TreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<bowflext216treadmill*>(detectedDevice)!=nullptr;
}
