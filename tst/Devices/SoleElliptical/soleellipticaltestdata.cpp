#include "soleellipticaltestdata.h" 
#include "soleelliptical.h"

SoleEllipticalTestData::SoleEllipticalTestData(): EllipticalTestData("Sole Elliptical") {
    this->addDeviceName("E95S", comparison::StartsWithIgnoreCase);
    this->addDeviceName("E25", comparison::StartsWithIgnoreCase);
    this->addDeviceName("E35", comparison::StartsWithIgnoreCase);
    this->addDeviceName("E55", comparison::StartsWithIgnoreCase);
    this->addDeviceName("E95", comparison::StartsWithIgnoreCase);
    this->addDeviceName("E98", comparison::StartsWithIgnoreCase);
    this->addDeviceName("XG400", comparison::StartsWithIgnoreCase);
    this->addDeviceName("E98S", comparison::StartsWithIgnoreCase);
}

deviceType SoleEllipticalTestData::get_expectedDeviceType() const { return deviceType::SoleElliptical; }

bool SoleEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<soleelliptical*>(detectedDevice)!=nullptr;
}
