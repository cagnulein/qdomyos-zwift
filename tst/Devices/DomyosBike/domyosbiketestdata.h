#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "domyosbike.h"

class DomyosBikeTestData : public BluetoothDeviceTestData {

public:
    DomyosBikeTestData() : BluetoothDeviceTestData("Domyos Bike") {

        this->addDeviceName("Domyos-Bike", comparison::StartsWith);
        this->addInvalidDeviceName("DomyosBridge", comparison::StartsWith);

    }


    deviceType get_expectedDeviceType() const override { return deviceType::DomyosBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyosbike*>(detectedDevice)!=nullptr;
    }
};

