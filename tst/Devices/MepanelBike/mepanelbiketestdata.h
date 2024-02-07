#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "mepanelbike.h"

class MepanelBikeTestData : public BluetoothDeviceTestData {

public:
    MepanelBikeTestData() : BluetoothDeviceTestData("Mepanel Bike") {
        this->addDeviceName("MEPANEL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::MepanelBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<mepanelbike*>(detectedDevice)!=nullptr;
    }
};

