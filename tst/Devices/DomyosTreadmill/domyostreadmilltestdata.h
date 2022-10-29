#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "domyostreadmill.h"

class DomyosTreadmillTestData : public BluetoothDeviceTestData {

public:
    DomyosTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::DomyosTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyostreadmill*>(detectedDevice)!=nullptr;
    }
};

