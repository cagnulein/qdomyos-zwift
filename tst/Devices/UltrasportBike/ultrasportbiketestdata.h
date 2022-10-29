#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "ultrasportbike.h"

class UltrasportBikeTestData : public BluetoothDeviceTestData {

public:
    UltrasportBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::UltrasportBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ultrasportbike*>(detectedDevice)!=nullptr;
    }
};

