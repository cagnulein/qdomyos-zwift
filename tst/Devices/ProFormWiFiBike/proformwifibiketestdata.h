#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformwifibike.h"

class ProFormWiFiBikeTestData : public BluetoothDeviceTestData {

public:
    ProFormWiFiBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformWifiBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformwifibike*>(detectedDevice)!=nullptr;
    }
};

