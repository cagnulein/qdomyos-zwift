#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "domyoselliptical.h"

class DomyosEllipticalTestData : public BluetoothDeviceTestData {

public:
    DomyosEllipticalTestData() {
        this->addDeviceName("Domyos-EL", comparison::StartsWith);
        this->addInvalidDeviceName("DomyosBridge",comparison::StartsWith);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::DomyosElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyoselliptical*>(detectedDevice)!=nullptr;
    }
};

