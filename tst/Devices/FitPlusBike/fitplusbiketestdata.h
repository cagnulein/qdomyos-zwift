#pragma once

#include "Devices/Bike/biketestdata.h"


class FitPlusBikeFSTestData : public BikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    FitPlusBikeFSTestData();


    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class FitPlusBikeMRKTestData : public BikeTestData {

public:
    FitPlusBikeMRKTestData();

    void configureExclusions() override;

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};
