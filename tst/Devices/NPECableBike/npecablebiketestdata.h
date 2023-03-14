#pragma once

#include "Devices/Bike/biketestdata.h"


class NPECableBikeTestData : public BikeTestData {
protected:
    NPECableBikeTestData(std::string testName);
public:
    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

class NPECableBike1TestData : public NPECableBikeTestData {

public:
    NPECableBike1TestData();

};

class NPECableBike2TestData : public NPECableBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    NPECableBike2TestData();
};
