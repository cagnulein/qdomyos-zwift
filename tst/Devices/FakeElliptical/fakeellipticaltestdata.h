#pragma once
#include "Devices/Elliptical/ellipticaltestdata.h"
#include "devices/fakeelliptical/fakeelliptical.h"

class FakeEllipticalTestData : public EllipticalTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.fakedevice_elliptical = enable;
        return true;
    }

 
public:
    FakeEllipticalTestData() : EllipticalTestData("Fake Elliptical") {
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakeelliptical*>(detectedDevice)!=nullptr;
    }
};

