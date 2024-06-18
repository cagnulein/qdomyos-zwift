#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/bkoolbike/bkoolbike.h"

class BkoolBikeTestData : public BluetoothDeviceTestData {

public:
    BkoolBikeTestData() : BluetoothDeviceTestData("Bkool Bike") {
        this->addDeviceName("BKOOLSMARTPRO", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::BkoolBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<bkoolbike*>(detectedDevice)!=nullptr;
    }
};

