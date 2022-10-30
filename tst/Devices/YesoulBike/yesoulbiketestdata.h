#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "yesoulbike.h"

class YesoulBikeTestData : public BluetoothDeviceTestData {

public:
    YesoulBikeTestData() {
        this->addDeviceName(yesoulbike::bluetoothName, comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::YesoulBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<yesoulbike*>(detectedDevice)!=nullptr;
    }
};

