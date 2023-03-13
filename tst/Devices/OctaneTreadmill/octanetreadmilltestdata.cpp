#include "octanetreadmilltestdata.h" 
#include "octanetreadmill.h"

OctaneTreadmillTestData::OctaneTreadmillTestData() : TreadmillTestData("Octane Treadmill") {
    this->addDeviceName("ZR7", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ZR8", comparison::StartsWithIgnoreCase);
}

deviceType OctaneTreadmillTestData::get_expectedDeviceType() const { return deviceType::OctaneTreadmill; }

bool OctaneTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<octanetreadmill*>(detectedDevice)!=nullptr;
}
