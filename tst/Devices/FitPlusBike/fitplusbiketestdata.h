#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fitplusbike.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"

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
    FitPlusBikeMRKTestData() {

        this->addDeviceName("MRK-", comparison::StartsWith);

        this->exclude(new FTMSBikeTestData());
        this->exclude(new SnodeBike1TestData());
        this->exclude(new SnodeBike2TestData());
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FitPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
    }
};
