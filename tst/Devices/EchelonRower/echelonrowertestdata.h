#pragma once

#include "Devices/Rower/rowertestdata.h"

class EchelonRowerTestData : public RowerTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const;
public:
    EchelonRowerTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

