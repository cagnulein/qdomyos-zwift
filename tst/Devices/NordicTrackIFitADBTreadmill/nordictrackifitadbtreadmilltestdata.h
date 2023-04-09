#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class NordicTrackIFitADBTreadmillTestData : public TreadmillTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    NordicTrackIFitADBTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;

    bool get_expectedLockscreenZwiftMode() const override;
};

