#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class TechnoGymMyRunTreadmillTestData : public TreadmillTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    TechnoGymMyRunTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

