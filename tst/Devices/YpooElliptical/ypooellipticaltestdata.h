#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "ypooelliptical.h"

class YpooEllipticalTestData : public BluetoothDeviceTestData {

public:
    YpooEllipticalTestData() : BluetoothDeviceTestData("Ypoo Elliptical") {
        this->addDeviceName("YPOO-U3-", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::YpooElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ypooelliptical*>(detectedDevice)!=nullptr;
    }
};
