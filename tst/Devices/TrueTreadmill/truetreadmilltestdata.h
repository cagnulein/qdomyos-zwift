#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/truetreadmill/truetreadmill.h"

class TrueTreadmillTestData : public BluetoothDeviceTestData {

public:
    TrueTreadmillTestData() : BluetoothDeviceTestData("True Treadmill") {
        this->addDeviceName("TRUE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TREADMILL", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ASSAULT TREADMILL ", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrueTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<truetreadmill*>(detectedDevice)!=nullptr;
    }
};

