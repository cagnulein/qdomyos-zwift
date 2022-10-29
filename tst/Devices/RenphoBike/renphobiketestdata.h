#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "renphobike.h"

class RenphoBikeTestData : public BluetoothDeviceTestData {

public:
    RenphoBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::RenphoBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<renphobike*>(detectedDevice)!=nullptr;
    }
};

