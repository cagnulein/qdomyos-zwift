#pragma once

#include "Devices/Bike/biketestdata.h"


class CSCBikeTestData : public BikeTestData {
protected:
    QString cscBikeName;
public:
    CSCBikeTestData(std::string testName);

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class CSCBike1TestData : public CSCBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    CSCBike1TestData();
};


class CSCBike2TestData : public CSCBikeTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    CSCBike2TestData();
};
