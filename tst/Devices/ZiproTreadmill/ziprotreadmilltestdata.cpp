#include "ziprotreadmilltestdata.h"
#include "ziprotreadmill.h"

ZiproTreadmillTestData::ZiproTreadmillTestData() : TreadmillTestData("Zipro Treadmill") {
    this->addDeviceName("RZ_TREADMIL", comparison::StartsWithIgnoreCase);
}

deviceType ZiproTreadmillTestData::get_expectedDeviceType() const { return deviceType::ZiproTreadmill; }

bool ZiproTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<ziprotreadmill*>(detectedDevice)!=nullptr;
}
