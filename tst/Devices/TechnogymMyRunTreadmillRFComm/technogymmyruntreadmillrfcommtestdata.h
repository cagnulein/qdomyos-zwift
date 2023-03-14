#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class TechnogymMyRunTreadmillRFCommTestData : public TreadmillTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    TechnogymMyRunTreadmillRFCommTestData();
    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

