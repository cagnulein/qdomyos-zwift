#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/toorxtreadmill/toorxtreadmill.h"

class ToorxTreadmillTestData : public BluetoothDeviceTestData {

public:
    ToorxTreadmillTestData() : BluetoothDeviceTestData("Toorx Treadmill") {
        this->addDeviceName("TRX ROUTE KEY", comparison::StartsWith);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::ToorxTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<toorxtreadmill*>(detectedDevice)!=nullptr;
    }
};

