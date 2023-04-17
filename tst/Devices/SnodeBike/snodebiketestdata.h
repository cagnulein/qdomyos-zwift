#pragma once

#include "Devices/Bike/biketestdata.h"



class SnodeBikeTestData : public BikeTestData {
protected:
    SnodeBikeTestData(std::string testName);

    void configureExclusions() override;
public:

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class SnodeBike1TestData : public SnodeBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    SnodeBike1TestData();
};

class SnodeBike2TestData : public SnodeBikeTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override;
public:
    SnodeBike2TestData();

};
