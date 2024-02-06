#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/elitesterzosmart/elitesterzosmart.h"

class EliteSterzoSmartTestData : public BluetoothDeviceTestData {

public:
    EliteSterzoSmartTestData() : BluetoothDeviceTestData("Elite Sterzo Smart") {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EliteSterzoSmart; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<elitesterzosmart*>(detectedDevice)!=nullptr;
    }
};

