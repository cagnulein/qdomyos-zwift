#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "paferstreadmill.h"

class PafersTreadmillTestData : public BluetoothDeviceTestData {

public:
    PafersTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<paferstreadmill*>(detectedDevice)!=nullptr;
    }
};

