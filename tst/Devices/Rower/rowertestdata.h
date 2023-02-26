#pragma once

#include "Devices/bluetoothdevicetestdata.h"


class RowerTestData : public BluetoothDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info,
                                     std::vector<LockscreenFunctionsTestData>& configurations) const override;
public:
    RowerTestData(std::string testName);

};

