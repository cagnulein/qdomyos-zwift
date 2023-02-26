#include "truetreadmilltestdata.h" 
#include "truetreadmill.h"

TrueTreadmillTestData::TrueTreadmillTestData() : TreadmillTestData("True Treadmill") {
    this->addDeviceName("TRUE", comparison::StartsWithIgnoreCase);
    this->addDeviceName("TREADMILL", comparison::StartsWithIgnoreCase);
}

deviceType TrueTreadmillTestData::get_expectedDeviceType() const { return deviceType::TrueTreadmill; }

bool TrueTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<truetreadmill*>(detectedDevice)!=nullptr;
}
