#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/fitplusbike/fitplusbike.h"

class FitPlusBikeFSTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.fitplus_bike = enable;
        return true;
    }
public:
    FitPlusBikeFSTestData() : BluetoothDeviceTestData("FitPlus Bike"){
        this->addDeviceName("FS-", comparison::StartsWith);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::FitPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
    }
};

class FitPlusBikeMRKTestData : public BluetoothDeviceTestData {

public:
    FitPlusBikeMRKTestData() : BluetoothDeviceTestData("FitPlus Bike (MRK, no settings)"){

        this->addDeviceName("MRK-", comparison::StartsWith);
    }

    void configureExclusions() override;

    deviceType get_expectedDeviceType() const override { return deviceType::FitPlusBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitplusbike*>(detectedDevice)!=nullptr;
    }
};
