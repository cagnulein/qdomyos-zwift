#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "kingsmithr2treadmill.h"

class KingsmithR2TreadmillTestData : public BluetoothDeviceTestData {

public:
    KingsmithR2TreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::KingsmithR2Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<kingsmithr2treadmill*>(detectedDevice)!=nullptr;
    }
};

