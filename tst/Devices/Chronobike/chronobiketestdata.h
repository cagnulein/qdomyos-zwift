#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/chronobike/chronobike.h"

class ChronobikeTestData : public BluetoothDeviceTestData {

public:
    ChronobikeTestData() : BluetoothDeviceTestData("Chronobike") {
        this->addDeviceName("CHRONO ", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ChronoBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<chronobike*>(detectedDevice)!=nullptr;
    }
};

