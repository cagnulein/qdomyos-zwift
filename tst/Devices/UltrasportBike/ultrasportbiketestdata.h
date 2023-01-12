#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "ultrasportbike.h"

class UltrasportBikeTestData : public BluetoothDeviceTestData {

public:
    UltrasportBikeTestData() : BluetoothDeviceTestData("Ultrasport Bike") {
        this->addDeviceName("X-BIKE", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::UltraSportBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ultrasportbike*>(detectedDevice)!=nullptr;
    }
};

