#pragma once

#include "Devices/Bike/biketestdata.h"


class PafersBikeTestData : public BikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    PafersBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

