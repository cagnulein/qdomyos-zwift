#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/schwinn170bike/schwinn170bike.h"

class Schwinn170BikeTestData : public BluetoothDeviceTestData {

public:
    Schwinn170BikeTestData() : BluetoothDeviceTestData("Schwinn 170 Bike") {

        this->addDeviceName("SCHWINN 170/270", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::Schwinn170Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<schwinn170bike*>(detectedDevice)!=nullptr;
    }
};

