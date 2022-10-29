#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "ftmsrower.h"

class FTMSRowerTestData : public BluetoothDeviceTestData {

public:
    FTMSRowerTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FTMSRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ftmsrower*>(detectedDevice)!=nullptr;
    }
};

