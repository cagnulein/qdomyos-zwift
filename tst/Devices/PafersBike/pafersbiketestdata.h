#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "pafersbike.h"

class PafersBikeTestData : public BluetoothDeviceTestData {

public:
    PafersBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::PafersBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<pafersbike*>(detectedDevice)!=nullptr;
    }
};

