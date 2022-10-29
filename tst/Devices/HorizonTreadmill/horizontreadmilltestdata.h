#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "horizontreadmill.h"

class HorizonTreadmillTestData : public BluetoothDeviceTestData {

public:
    HorizonTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<horizontreadmill*>(detectedDevice)!=nullptr;
    }
};

