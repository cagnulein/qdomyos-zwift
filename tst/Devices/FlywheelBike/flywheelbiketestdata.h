#pragma once

#include "Devices/Bike/biketestdata.h"


class FlywheelBikeTestData : public BikeTestData {

protected:
    FlywheelBikeTestData(std::string testName);
public:
    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};


class FlywheelBike1TestData : public FlywheelBikeTestData {

public:
    FlywheelBike1TestData();

};

class FlywheelBike2TestData : public FlywheelBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    FlywheelBike2TestData();
};
