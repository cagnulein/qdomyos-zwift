#pragma once

#include "VirtualDevices/virtualdevicetestdata.h"

class VirtualRowerTestData : public VirtualDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info, std::vector<LockscreenFunctionsTestData>& configurations) const override;
public:
    VirtualRowerTestData();

    virtualdevice * createDevice(bluetoothdevice * device) override;
};
