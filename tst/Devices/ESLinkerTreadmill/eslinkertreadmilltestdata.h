#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "eslinkertreadmill.h"

class ESLinkerTreadmillTestData : public BluetoothDeviceTestData {

public:
    ESLinkerTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ESLinkerTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<eslinkertreadmill*>(detectedDevice)!=nullptr;
    }
};

