#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fitplusbike.h"

class FitPlusBikeTestData : public BluetoothDeviceTestData {

public:
    FitPlusBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FitPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
    }
};

