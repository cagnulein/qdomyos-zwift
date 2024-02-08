#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/eslinkertreadmill/eslinkertreadmill.h"

class ESLinkerTreadmillTestData : public BluetoothDeviceTestData {

public:
    ESLinkerTreadmillTestData(): BluetoothDeviceTestData("ES Linker Treadmill") {
        this->addDeviceName("ESLINKER", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ESLinkerTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<eslinkertreadmill*>(detectedDevice)!=nullptr;
    }
};

