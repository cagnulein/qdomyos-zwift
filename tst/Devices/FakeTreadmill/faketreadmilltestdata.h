#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"

class FakeTreadmillTestData : public TreadmillTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    FakeTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

