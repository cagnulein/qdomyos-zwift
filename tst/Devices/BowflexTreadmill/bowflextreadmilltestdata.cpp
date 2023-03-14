#include "bowflextreadmilltestdata.h" 
#include "bowflextreadmill.h"

BowflexTreadmillTestData::BowflexTreadmillTestData() : TreadmillTestData("Bowflex Treadmill"){}

QStringList BowflexTreadmillTestData::get_deviceNames() const {
    QStringList result;

    return result;
}

deviceType BowflexTreadmillTestData::get_expectedDeviceType() const {
    // device not supported
    return deviceType::None;
}

bool BowflexTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<bowflextreadmill*>(detectedDevice)!=nullptr;
}
