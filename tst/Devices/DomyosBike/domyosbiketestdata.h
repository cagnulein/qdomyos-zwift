#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "domyosbike.h"

class DomyosBikeTestData : public BluetoothDeviceTestData {

public:
    DomyosBikeTestData() {

        this->addDeviceName("Domyos-Bike", false, true);

        this->addInvalidDeviceName("DomyosBridge");
        this->addInvalidDeviceName("DomyosBridgeX");
    }


    deviceType get_expectedDeviceType() const override { return deviceType::DomyosBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyosbike*>(detectedDevice)!=nullptr;
    }
};

