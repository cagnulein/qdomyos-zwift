#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformwifibike.h"

class ProFormWiFiBikeTestData : public BluetoothDeviceTestData {

public:
    ProFormWiFiBikeTestData() {
        this->hasSettings= true;

        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.proformtdf4ip = enable ? this->get_testIP():QString();
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformWifiBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformwifibike*>(detectedDevice)!=nullptr;
    }
};

