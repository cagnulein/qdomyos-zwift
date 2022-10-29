#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "tacxneo2.h"

class TacxNeo2TestData : public BluetoothDeviceTestData {

public:
    TacxNeo2TestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TacxNeo2; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<tacxneo2*>(detectedDevice)!=nullptr;
    }
};

