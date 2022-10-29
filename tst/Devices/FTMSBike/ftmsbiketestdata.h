#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "ftmsbike.h"

class FTMSBikeTestData : public BluetoothDeviceTestData {

public:
    FTMSBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FTMSBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ftmsbike*>(detectedDevice)!=nullptr;
    }
};

