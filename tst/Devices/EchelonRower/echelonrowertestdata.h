#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "echelonrower.h"

class EchelonRowerTestData : public BluetoothDeviceTestData {

public:
    EchelonRowerTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonrower*>(detectedDevice)!=nullptr;
    }
};

