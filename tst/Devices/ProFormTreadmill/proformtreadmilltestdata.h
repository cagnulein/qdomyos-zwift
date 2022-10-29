#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformtreadmill.h"

class ProFormTreadmillTestData : public BluetoothDeviceTestData {

public:
    ProFormTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProFormTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformtreadmill*>(detectedDevice)!=nullptr;
    }
};

