#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class PafersTreadmillTestData : public TreadmillTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;


public:
    PafersTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

