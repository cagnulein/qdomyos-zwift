#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "echelonstride.h"

class EchelonStrideTreadmillTestData : public BluetoothDeviceTestData {

public:
    EchelonStrideTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonStride; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonstride*>(detectedDevice)!=nullptr;
    }
};

