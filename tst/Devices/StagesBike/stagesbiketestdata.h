#pragma once

#include "Devices/Bike/biketestdata.h"


class StagesBikeTestData : public BikeTestData {
protected:
    StagesBikeTestData(std::string testName);

    void configureExclusions() override;
public:

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class StagesBike1TestData : public StagesBikeTestData {

public:
    StagesBike1TestData();

};


class StagesBike2TestData : public StagesBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    StagesBike2TestData();
};
