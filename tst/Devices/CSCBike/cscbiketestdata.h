#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "cscbike.h"

class CSCBikeTestData : public BluetoothDeviceTestData {

public:
    CSCBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::CSCBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<cscbike*>(detectedDevice)!=nullptr;
    }
};

