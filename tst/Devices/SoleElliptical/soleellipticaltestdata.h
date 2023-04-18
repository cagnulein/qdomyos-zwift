#pragma once
#include "Devices/Elliptical/ellipticaltestdata.h"
#include "Devices/bluetoothdevicetestdata.h"
#include "soleelliptical.h"

class SoleEllipticalTestData : public EllipticalTestData {

public:
    SoleEllipticalTestData(): EllipticalTestData("Sole Elliptical") {
        this->addDeviceName("E95S", comparison::StartsWithIgnoreCase);
        this->addDeviceName("E25", comparison::StartsWithIgnoreCase);
        this->addDeviceName("E35", comparison::StartsWithIgnoreCase);
        this->addDeviceName("E55", comparison::StartsWithIgnoreCase);
        this->addDeviceName("E95", comparison::StartsWithIgnoreCase);
        this->addDeviceName("E98", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XG400", comparison::StartsWithIgnoreCase);
        this->addDeviceName("E98S", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SoleElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<soleelliptical*>(detectedDevice)!=nullptr;
    }
};

