#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fitplusbike.h"

class FitPlusBikeFSTestData : public BluetoothDeviceTestData {

public:
    FitPlusBikeFSTestData() {
        this->addDeviceName("FS-", comparison::StartsWith);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.fitplus_bike = enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FitPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
    }
};

class FitPlusBikeMRKTestData : public BluetoothDeviceTestData {

public:
    FitPlusBikeMRKTestData();

    deviceType get_expectedDeviceType() const override { return deviceType::FitPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
    }
};
