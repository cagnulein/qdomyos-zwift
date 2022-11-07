#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformrower.h"

class ProFormRowerTestData : public BluetoothDeviceTestData {

public:
    ProFormRowerTestData() : BluetoothDeviceTestData("ProForm Rower") {
        this->addDeviceName("I_RW", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformrower*>(detectedDevice)!=nullptr;
    }
};

