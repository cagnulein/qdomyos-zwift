#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "chronobike.h"

class ChronobikeTestData : public BluetoothDeviceTestData {

public:
    ChronobikeTestData() {
        this->addDeviceName("CHRONO ", true, true);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ChronoBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<chronobike*>(detectedDevice)!=nullptr;
    }
};

