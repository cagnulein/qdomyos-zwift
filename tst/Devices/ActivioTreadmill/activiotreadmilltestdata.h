#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "activiotreadmill.h"

class ActivioTreadmillTestData : public BluetoothDeviceTestData {

public:
    ActivioTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        result.append("RUNNERT");
        result.append("runnert");
        result.append("ruNnerT");
        result.append("runnERt12314");

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ActivioTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<activiotreadmill*>(detectedDevice)!=nullptr;
    }
};

