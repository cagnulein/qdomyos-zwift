#pragma once


#include "Devices/Bike/biketestdata.h"


#include "devices/flywheelbike/flywheelbike.h"


class FlywheelBikeTestData : public BikeTestData {

protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new flywheelbike(options.noResistance, options.noHeartService);
    }
    FlywheelBikeTestData(std::string testName) : BikeTestData(testName) {
    }
public:
    deviceType get_expectedDeviceType() const override { return deviceType::FlywheelBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<flywheelbike*>(detectedDevice)!=nullptr;
    }
};


class FlywheelBike1TestData : public FlywheelBikeTestData {

public:
    FlywheelBike1TestData() : FlywheelBikeTestData("Flywheel Bike") {
        this->addDeviceName("Flywheel", comparison::StartsWith);
    }

};

class FlywheelBike2TestData : public FlywheelBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.flywheel_life_fitness_ic8 = enable;
        return true;
    }
public:
    FlywheelBike2TestData() : FlywheelBikeTestData("Flywheel Life Fitness IC8") {
        // BIKE 1, BIKE 2, BIKE 3...
        this->addDeviceName("BIKE", comparison::StartsWithIgnoreCase, 6);
    }
};
