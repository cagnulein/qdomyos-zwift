#pragma once

#include "Devices/Bike/biketestdata.h"


class FTMSBikeTestData : public BikeTestData {
protected:
    void configureExclusions() override;

    FTMSBikeTestData(std::string testName);
public:

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class FTMSBike1TestData : public FTMSBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:

    FTMSBike1TestData();


};

class FTMSBike2TestData : public FTMSBikeTestData {
public:

    FTMSBike2TestData();
};

class FTMSBike3TestData : public FTMSBikeTestData {
private:
    QString ftmsAccessoryName;
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    FTMSBike3TestData();
};
