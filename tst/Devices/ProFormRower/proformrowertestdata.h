#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformrower.h"

class ProFormRowerTestData : public BluetoothDeviceTestData {

public:
    ProFormRowerTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProFormRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformrower*>(detectedDevice)!=nullptr;
    }
};

