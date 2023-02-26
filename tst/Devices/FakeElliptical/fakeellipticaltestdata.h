#pragma once

#include "Devices/Elliptical/ellipticaltestdata.h"

class FakeEllipticalTestData : public EllipticalTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    FakeEllipticalTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

