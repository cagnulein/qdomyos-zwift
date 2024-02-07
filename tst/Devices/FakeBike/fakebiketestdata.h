#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fakebike.h"

class FakeBikeTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.fake_bike = enable;
        return true;
    }
public:
    FakeBikeTestData() : BluetoothDeviceTestData("Fake Bike"){
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakebike*>(detectedDevice)!=nullptr;
    }
};

