#pragma once

#include "Devices/Bike/biketestdata.h"
#include "devices/renphobike/renphobike.h"

class RenphoBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new renphobike(options.noResistance, options.noHeartService);
    }

    RenphoBikeTestData(std::string testName) : BikeTestData(testName) {
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
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        DeviceDiscoveryInfo config(info);

        config.toorx_ftms = enable;
        config.toorx_bike = true;
        configurations.push_back(config);
        config.toorx_ftms = enable;
        config.toorx_bike = false;
        configurations.push_back(config);
    }
public:
    RenphoBike2TestData() : RenphoBikeTestData("Renpho Bike (Toorx FTMS)"){
        this->addDeviceName("TOORX", comparison::StartsWith);
    }

};
