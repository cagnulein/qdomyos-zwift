#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class FakeEllipticalTestData : public EllipticalTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const override;
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    FakeEllipticalTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

