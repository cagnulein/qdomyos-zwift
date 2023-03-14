#pragma once

#include "VirtualDevices/virtualdevicetestdata.h"

class VirtualBikeTestData : public VirtualDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info, std::vector<LockscreenFunctionsTestData>& configurations) const override;
public:
    VirtualBikeTestData();
    virtualdevice * createDevice(bluetoothdevice * device) override;
};

