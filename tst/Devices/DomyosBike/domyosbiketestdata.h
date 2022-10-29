#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "domyosbike.h"

class DomyosBikeTestData : public BluetoothDeviceTestData {

public:
    DomyosBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::DomyosBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyosbike*>(detectedDevice)!=nullptr;
    }
};

