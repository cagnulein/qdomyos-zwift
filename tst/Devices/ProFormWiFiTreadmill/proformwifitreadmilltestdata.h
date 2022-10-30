#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformwifitreadmill.h"

class ProFormWiFiTreadmillTestData : public BluetoothDeviceTestData {

public:
    ProFormWiFiTreadmillTestData() {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.proformtreadmillip = enable ? this->get_testIP():QString();
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformWifiTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformwifitreadmill*>(detectedDevice)!=nullptr;
    }
};

