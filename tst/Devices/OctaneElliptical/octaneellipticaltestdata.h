#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "octaneelliptical.h"

class OctaneEllipticalTestData : public BluetoothDeviceTestData {

public:
    OctaneEllipticalTestData() : BluetoothDeviceTestData("Octane Elliptical") {
        this->addDeviceName("Q37", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::OctaneElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<octaneelliptical*>(detectedDevice)!=nullptr;
    }
};

