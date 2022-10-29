#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "stagesbike.h"

class StagesBikeTestData : public BluetoothDeviceTestData {

public:
    StagesBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::StagesBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<stagesbike*>(detectedDevice)!=nullptr;
    }
};

