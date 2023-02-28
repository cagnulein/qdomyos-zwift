#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "apexbike.h"

class ApexBikeTestData : public BluetoothDeviceTestData {

public:
    EchelonConnectSportBikeTestData() : BluetoothDeviceTestData("Apex Bike") {
        this->addDeviceName("WLT8266BM", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ApexBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<apexbike*>(detectedDevice)!=nullptr;
    }
};

