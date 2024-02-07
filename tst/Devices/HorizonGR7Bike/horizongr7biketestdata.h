#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "horizongr7bike.h"

class HorizonGR7BikeTestData : public BluetoothDeviceTestData {

public:
    HorizonGR7BikeTestData() : BluetoothDeviceTestData("Horizon GR7 Bike") {
        this->addDeviceName("JFIC", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonGr7Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<horizongr7bike*>(detectedDevice)!=nullptr;
    }
};

