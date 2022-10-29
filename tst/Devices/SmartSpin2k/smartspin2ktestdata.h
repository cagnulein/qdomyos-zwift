#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "smartspin2k.h"

class SmartSpin2kTestData : public BluetoothDeviceTestData {

public:
    SmartSpin2kTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SmartSpin2k; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<smartspin2k*>(detectedDevice)!=nullptr;
    }
};

