#include "octaneellipticaltestdata.h" 
#include "octaneelliptical.h"

OctaneEllipticalTestData::OctaneEllipticalTestData() : EllipticalTestData("Octane Treadmill") {
    this->addDeviceName("Q37", comparison::StartsWithIgnoreCase);
}

deviceType OctaneEllipticalTestData::get_expectedDeviceType() const { return deviceType::OctaneElliptical; }

bool OctaneEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<octaneelliptical*>(detectedDevice)!=nullptr;
}
