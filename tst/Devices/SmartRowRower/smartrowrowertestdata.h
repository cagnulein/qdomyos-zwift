#pragma once

#include "Devices/Rower/rowertestdata.h"


class SmartRowRowerTestData : public RowerTestData {
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const override;
public:
    SmartRowRowerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

