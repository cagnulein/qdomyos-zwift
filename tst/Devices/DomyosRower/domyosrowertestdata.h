#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "domyosrower.h"

class DomyosRowerTestData : public BluetoothDeviceTestData {

public:
    DomyosRowerTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::DomyosRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyosrower*>(detectedDevice)!=nullptr;
    }
};

