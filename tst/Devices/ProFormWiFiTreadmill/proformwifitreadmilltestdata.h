#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"

#include "devices/proformwifitreadmill/proformwifitreadmill.h"


class ProFormWiFiTreadmillTestData : public TreadmillTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.proformtreadmillip = enable ? this->get_testIP():QString();
        return true;
    }
public:
    ProFormWiFiTreadmillTestData() : TreadmillTestData("ProForm WiFi Treadmill") {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformWifiTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformwifitreadmill*>(detectedDevice)!=nullptr;
    }
};

