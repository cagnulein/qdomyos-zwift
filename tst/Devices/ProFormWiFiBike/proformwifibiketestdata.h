#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/proformwifibike/proformwifibike.h"

class ProFormWiFiBikeTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.proformtdf4ip = enable ? this->get_testIP():QString();
        return true;
    }
public:
    ProFormWiFiBikeTestData(): BluetoothDeviceTestData("ProForm WiFi Bike") {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformWifiBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformwifibike*>(detectedDevice)!=nullptr;
    }
};

