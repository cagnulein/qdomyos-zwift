#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/domyosrower/domyosrower.h"

class DomyosRowerTestData : public BluetoothDeviceTestData {

public:
    DomyosRowerTestData() : BluetoothDeviceTestData("Domyos Rower") {
        this->addDeviceName("DOMYOS-ROW", comparison::StartsWithIgnoreCase);
        this->addInvalidDeviceName("DomyosBridge", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::DomyosRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyosrower*>(detectedDevice)!=nullptr;
    }
};

