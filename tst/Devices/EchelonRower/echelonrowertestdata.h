#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "echelonrower.h"

class EchelonRowerTestData : public BluetoothDeviceTestData {

public:
    EchelonRowerTestData() {
        this->addDeviceName("ECH-ROW", comparison::StartsWith);
        this->addDeviceName("ROW-S", comparison::StartsWith);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::EchelonRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonrower*>(detectedDevice)!=nullptr;
    }
};

