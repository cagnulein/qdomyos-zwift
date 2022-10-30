#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "bhfitnesselliptical.h"

class BHFitnessEllipticalTestData : public BluetoothDeviceTestData {

public:
    BHFitnessEllipticalTestData() {
        this->addDeviceName("B01_", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::BHFitnessElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
          return dynamic_cast<bhfitnesselliptical*>(detectedDevice)!=nullptr;
    }
};
