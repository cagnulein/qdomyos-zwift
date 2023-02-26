#pragma once

#include "Devices/bluetoothdevicetestdata.h"


class EllipticalTestData : public BluetoothDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info,
                                     std::vector<LockscreenFunctionsTestData>& configurations) const override;
public:
    EllipticalTestData(std::string testName);


};

