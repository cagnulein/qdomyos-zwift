#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "faketreadmill.h"

class FakeTreadmillTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.fakedevice_treadmill = enable;
        return true;
    }
public:
    FakeTreadmillTestData(): BluetoothDeviceTestData("Fake Treadmill") {
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }   

    deviceType get_expectedDeviceType() const override { return deviceType::FakeTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<faketreadmill*>(detectedDevice)!=nullptr;
    }
};

