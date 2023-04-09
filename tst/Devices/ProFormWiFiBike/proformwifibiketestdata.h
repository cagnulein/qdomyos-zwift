#pragma once

#include "Devices/Bike/biketestdata.h"


class ProFormWiFiBikeTestData : public BikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override;
public:
    ProFormWiFiBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;

};

