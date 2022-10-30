#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "iconceptbike.h"

class iConceptBikeTestData : public BluetoothDeviceTestData {

public:
    iConceptBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::IConceptBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<iconceptbike*>(detectedDevice)!=nullptr;
    }
};

