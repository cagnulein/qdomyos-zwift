#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformwifitreadmill.h"

class ProFormWiFiTreadmillTestData : public BluetoothDeviceTestData {

public:
    ProFormWiFiTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProFormWiFiTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformwifitreadmill*>(detectedDevice)!=nullptr;
    }
};

