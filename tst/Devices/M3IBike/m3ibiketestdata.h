#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "m3ibike.h"

class M3IBikeTestData : public BluetoothDeviceTestData {

public:
    M3IBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::M3IBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<m3ibike*>(detectedDevice)!=nullptr;
    }
};

