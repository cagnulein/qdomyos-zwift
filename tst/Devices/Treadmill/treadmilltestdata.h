#pragma once

#include "Devices/bluetoothdevicetestdata.h"


class TreadmillTestData : public BluetoothDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info,
                                     std::vector<LockscreenFunctionsTestData>& configurations) const override;

public:
    TreadmillTestData(std::string testName);

};

