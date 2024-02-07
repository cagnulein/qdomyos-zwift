#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/fakeelliptical/fakeelliptical.h"

class FakeEllipticalTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.fakedevice_elliptical = enable;
        return true;
    }
public:
    FakeEllipticalTestData() : BluetoothDeviceTestData("Fake Elliptical") {
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakeelliptical*>(detectedDevice)!=nullptr;
    }
};

