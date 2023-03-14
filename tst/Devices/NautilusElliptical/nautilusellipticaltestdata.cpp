#include "nautilusellipticaltestdata.h"
#include "nautiluselliptical.h"

NautilusEllipticalTestData::NautilusEllipticalTestData(): EllipticalTestData("Nautilus Elliptical") {
    this->addDeviceName("NAUTILUS E", comparison::StartsWithIgnoreCase);
}

deviceType NautilusEllipticalTestData::get_expectedDeviceType() const { return deviceType::NautilusElliptical; }

bool NautilusEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<nautiluselliptical*>(detectedDevice)!=nullptr;
}
