#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "eliterizer.h"

class EliteRizerTestData : public BluetoothDeviceTestData {

public:
    EliteRizerTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EliteRizer; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<eliterizer*>(detectedDevice)!=nullptr;
    }
};

