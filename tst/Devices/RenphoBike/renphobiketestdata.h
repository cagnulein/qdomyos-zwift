#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"

#include "renphobike.h"

class RenphoBikeTestData : public BluetoothDeviceTestData {
protected:
    RenphoBikeTestData() {
    }

    void configureExclusions() override;
public:   
    deviceType get_expectedDeviceType() const override { return deviceType::RenphoBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<renphobike*>(detectedDevice)!=nullptr;
    }
};

class RenphoBike1TestData : public RenphoBikeTestData {
public:
    RenphoBike1TestData(){
        this->addDeviceName("RQ", comparison::StartsWithIgnoreCase, 5);
        this->addDeviceName("SCH130", comparison::StartsWithIgnoreCase);
    }
};

class RenphoBike2TestData : public RenphoBikeTestData {
public:
    RenphoBike2TestData(){
        this->hasSettings = true;
        this->addDeviceName("TOORX", comparison::StartsWith);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.toorx_ftms = enable;
        info.toorx_bike = true;
    }
};
