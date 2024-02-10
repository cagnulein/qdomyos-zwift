#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/ziprotreadmill/ziprotreadmill.h"

class ZiproTreadmillTestData : public BluetoothDeviceTestData {

public:
    ZiproTreadmillTestData() : BluetoothDeviceTestData("Zipro Treadmill") {
        this->addDeviceName("RZ_TREADMIL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ZiproTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ziprotreadmill*>(detectedDevice)!=nullptr;
    }
};

