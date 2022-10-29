#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "heartratebelt.h"

class HeartRateBeltTestData : public BluetoothDeviceTestData {

public:
    HeartRateBeltTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HeartRateBelt; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<heartratebelt*>(detectedDevice)!=nullptr;
    }
};

