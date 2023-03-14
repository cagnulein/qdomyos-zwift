#include "proformtreadmilltestdata.h" 
#include "proformtreadmill.h"

ProFormTreadmillTestData::ProFormTreadmillTestData(): TreadmillTestData("ProForm Treadmill") {
    this->addDeviceName("I_TL", comparison::StartsWith);
}

deviceType ProFormTreadmillTestData::get_expectedDeviceType() const { return deviceType::ProformTreadmill; }

bool ProFormTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<proformtreadmill*>(detectedDevice)!=nullptr;
}
