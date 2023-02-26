#include "fakeellipticaltestdata.h" 
#include "fakeelliptical.h"

bool FakeEllipticalTestData::configureSettings(DeviceDiscoveryInfo &info, bool enable) const {
    info.fakedevice_elliptical = enable;
    return true;
}

FakeEllipticalTestData::FakeEllipticalTestData() : EllipticalTestData("Fake Elliptical") {
    this->addDeviceName("", comparison::StartsWithIgnoreCase);
}

deviceType FakeEllipticalTestData::get_expectedDeviceType() const { return deviceType::FakeElliptical; }

bool FakeEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<fakeelliptical*>(detectedDevice)!=nullptr;
}
