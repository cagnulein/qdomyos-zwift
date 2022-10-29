#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "kingsmithr1protreadmill.h"

class KingsmithR1ProTreadmillTestData : public BluetoothDeviceTestData {

public:
    KingsmithR1ProTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::KingsmithR1ProTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<kingsmithr1protreadmill*>(detectedDevice)!=nullptr;
    }
};

