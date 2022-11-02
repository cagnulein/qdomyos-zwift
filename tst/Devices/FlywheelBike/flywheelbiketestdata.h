#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "flywheelbike.h"

class FlywheelBikeTestData : public BluetoothDeviceTestData {

protected:
    FlywheelBikeTestData() {
    }
public:
    deviceType get_expectedDeviceType() const override { return deviceType::FlywheelBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<flywheelbike*>(detectedDevice)!=nullptr;
    }
};


class FlywheelBike1TestData : public FlywheelBikeTestData {

public:
    FlywheelBike1TestData() {
        this->addDeviceName("Flywheel", comparison::StartsWith);
    }

};

class FlywheelBike2TestData : public FlywheelBikeTestData {
protected:
    bool configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.flywheel_life_fitness_ic8 = enable;
        return true;
    }
public:
    FlywheelBike2TestData() {
        // BIKE 1, BIKE 2, BIKE 3...
        this->addDeviceName("BIKE", comparison::StartsWithIgnoreCase, 6);
    }
};
