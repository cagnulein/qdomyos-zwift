#pragma once

#include "Devices/Rower/rowertestdata.h"

class Concept2SkiErgTestData : public RowerTestData {
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info,
                                     std::vector<LockscreenFunctionsTestData>& configurations) const override;
public:
    Concept2SkiErgTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

