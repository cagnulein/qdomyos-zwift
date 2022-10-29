#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "truetreadmill.h"

class TrueTreadmillTestData : public BluetoothDeviceTestData {

public:
    TrueTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrueTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<truetreadmill*>(detectedDevice)!=nullptr;
    }
};

