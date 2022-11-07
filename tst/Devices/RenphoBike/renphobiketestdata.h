#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"

#include "renphobike.h"

class RenphoBikeTestData : public BluetoothDeviceTestData {
protected:
    RenphoBikeTestData(std::string testName) : BluetoothDeviceTestData(testName) {
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
    RenphoBike1TestData() : RenphoBikeTestData("Renpho Bike") {
        this->addDeviceName("RQ", comparison::StartsWithIgnoreCase, 5);
        this->addDeviceName("SCH130", comparison::StartsWithIgnoreCase);
    }
};

class RenphoBike2TestData : public RenphoBikeTestData {
protected:
    void configureSettings(const devicediscoveryinfo& info, bool enable, std::vector<devicediscoveryinfo> configurations) const override {
        devicediscoveryinfo info1(info);
        info1.toorx_ftms = enable;
        info1.toorx_bike = true;
        configurations.push_back(info1);

        devicediscoveryinfo info2(info);
        info2.toorx_ftms = enable;
        info2.toorx_bike = false;
        configurations.push_back(info2);
    }
public:
    RenphoBike2TestData() : RenphoBikeTestData("Renpho Bike (Toorx FTMS)"){
        this->addDeviceName("TOORX", comparison::StartsWith);
    }

};
