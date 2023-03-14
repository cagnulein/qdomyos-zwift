#pragma once

#include "Devices/Bike/biketestdata.h"


class TrxAppGateUSBBikeTestData : public BikeTestData {
protected:
    TrxAppGateUSBBikeTestData(std::string testName);

    void configureExclusions() override;
public:
    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class TrxAppGateUSBBike1TestData : public TrxAppGateUSBBikeTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    TrxAppGateUSBBike1TestData();

};

class TrxAppGateUSBBike2TestData : public TrxAppGateUSBBikeTestData {
protected:

    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    TrxAppGateUSBBike2TestData();
};
