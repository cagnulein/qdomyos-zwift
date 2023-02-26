#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class FitshowTreadmillTestData : public TreadmillTestData {

protected:

    FitshowTreadmillTestData(std::string testName);

public:

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};


class FitshowTreadmillFSTestData : public FitshowTreadmillTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    FitshowTreadmillFSTestData();

    void configureExclusions() override;
};


class FitshowTreadmillSWTestData : public FitshowTreadmillTestData {
public:
    FitshowTreadmillSWTestData();
};

class FitshowTreadmillBFTestData : public FitshowTreadmillTestData {
public:
    FitshowTreadmillBFTestData() : FitshowTreadmillTestData("FitShow BF"){
        this->addDeviceName("BF70", comparison::StartsWith);
    }
};
