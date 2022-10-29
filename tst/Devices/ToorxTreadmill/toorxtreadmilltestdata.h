#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "toorxtreadmill.h"

class ToorxTreadmillTestData : public BluetoothDeviceTestData {

public:
    ToorxTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ToorxTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<toorxtreadmill*>(detectedDevice)!=nullptr;
    }
};

