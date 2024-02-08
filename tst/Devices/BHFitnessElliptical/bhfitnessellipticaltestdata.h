#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/bhfitnesselliptical/bhfitnesselliptical.h"

class BHFitnessEllipticalTestData : public BluetoothDeviceTestData {

public:
    BHFitnessEllipticalTestData() : BluetoothDeviceTestData("BH Fitness Elliptical") {
        this->addDeviceName("B01_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::BHFitnessElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
          return dynamic_cast<bhfitnesselliptical*>(detectedDevice)!=nullptr;
    }
};
