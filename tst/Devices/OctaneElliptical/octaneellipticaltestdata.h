#pragma once
#include "Devices/Elliptical/ellipticaltestdata.h"
#include "Devices/bluetoothdevicetestdata.h"
#include "octaneelliptical.h"

class OctaneEllipticalTestData : public EllipticalTestData {

public:
    OctaneEllipticalTestData() : EllipticalTestData("Octane Elliptical") {
        this->addDeviceName("Q37", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::OctaneElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<octaneelliptical*>(detectedDevice)!=nullptr;
    }
};

