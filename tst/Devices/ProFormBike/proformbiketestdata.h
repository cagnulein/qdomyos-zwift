#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformbike.h"

class ProFormBikeTestData : public BluetoothDeviceTestData {

public:
    ProFormBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformbike*>(detectedDevice)!=nullptr;
    }
};

