#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "echelonstridetreadmill.h"

class EchelonStrideTreadmillTestData : public BluetoothDeviceTestData {

public:
    EchelonStrideTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonStrideTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonstridetreadmill*>(detectedDevice)!=nullptr;
    }
};

