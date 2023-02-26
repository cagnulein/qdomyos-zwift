#include "lifefitnesstreadmilltestdata.h"
#include "lifefitnesstreadmill.h"

LifeFitnessTreadmillTestData::LifeFitnessTreadmillTestData() : TreadmillTestData("Life Fitness Treadmill") {
    this->addDeviceName("LF", comparison::StartsWithIgnoreCase, 18);
}

deviceType LifeFitnessTreadmillTestData::get_expectedDeviceType() const { return deviceType::LifeFitnessTreadmill; }

bool LifeFitnessTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<lifefitnesstreadmill*>(detectedDevice)!=nullptr;
}
