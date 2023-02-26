#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class TrxAppGateUSBTreadmillTestData : public TreadmillTestData {

protected:
    void configureExclusions() override;

    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override;
public:
    TrxAppGateUSBTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

