#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"

#include "devices/focustreadmill/focustreadmill.h"


class FocusTreadmillTestData : public TreadmillTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        DeviceDiscoveryInfo config(info);
    }


public:
    FocusTreadmillTestData() : TreadmillTestData("Focus Treadmill") {
        this->addDeviceName("EW-TM-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FocusTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<focustreadmill*>(detectedDevice)!=nullptr;
    }
};

