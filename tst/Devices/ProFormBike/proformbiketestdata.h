#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/proformbike/proformbike.h"

class ProFormBikeTestData : public BluetoothDeviceTestData {

public:
    ProFormBikeTestData() : BluetoothDeviceTestData("ProForm Bike") {
        this->addDeviceName("I_EB", comparison::StartsWith);
        this->addDeviceName("I_SB", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformbike*>(detectedDevice)!=nullptr;
    }
};

