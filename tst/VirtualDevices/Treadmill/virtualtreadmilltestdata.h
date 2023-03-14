#pragma once

#include "VirtualDevices/virtualdevicetestdata.h"

class VirtualTreadmillTestData : public VirtualDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info, std::vector<LockscreenFunctionsTestData>& configurations) const override;
public:
    VirtualTreadmillTestData();

    virtualdevice * createDevice(bluetoothdevice * device) override;
};
