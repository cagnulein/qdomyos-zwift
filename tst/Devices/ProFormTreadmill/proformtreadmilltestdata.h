#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformtreadmill.h"

class ProFormTreadmillTestData : public BluetoothDeviceTestData {

public:
    ProFormTreadmillTestData(): BluetoothDeviceTestData("ProForm Treadmill") {
        this->addDeviceName("I_TL", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformtreadmill*>(detectedDevice)!=nullptr;
    }
};

