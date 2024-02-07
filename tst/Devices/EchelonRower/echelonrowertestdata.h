#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/echelonrower/echelonrower.h"

class EchelonRowerTestData : public BluetoothDeviceTestData {

public:
    EchelonRowerTestData() : BluetoothDeviceTestData("Echelon Rower") {
        this->addDeviceName("ECH-ROW", comparison::StartsWith);
        this->addDeviceName("ROWSPORT-", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ROW-S", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonrower*>(detectedDevice)!=nullptr;
    }
};

