#include "activiotreadmilltestdata.h" 
#include "activiotreadmill.h"

ActivioTreadmillTestData::ActivioTreadmillTestData() : TreadmillTestData("Activio Treadmill") {
    this->addDeviceName("RUNNERT", comparison::StartsWithIgnoreCase);
}

deviceType ActivioTreadmillTestData::get_expectedDeviceType() const { return deviceType::ActivioTreadmill; }

bool ActivioTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<activiotreadmill*>(detectedDevice)!=nullptr;
}
