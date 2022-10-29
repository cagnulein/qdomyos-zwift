#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "schwinnic4bike.h"

class SchwinnIC4BikeTestData : public BluetoothDeviceTestData {

public:
    SchwinnIC4BikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SchwinnIC4Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<schwinnic4bike*>(detectedDevice)!=nullptr;
    }
};

