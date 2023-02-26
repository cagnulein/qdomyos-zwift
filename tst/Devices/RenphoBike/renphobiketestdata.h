#pragma once

#include "Devices/Bike/biketestdata.h"


class RenphoBikeTestData : public BikeTestData {
protected:
    RenphoBikeTestData(std::string testName);

    void configureExclusions() override;
public:   
    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class RenphoBike1TestData : public RenphoBikeTestData {
public:
    RenphoBike1TestData();
};

class RenphoBike2TestData : public RenphoBikeTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    RenphoBike2TestData();

};
