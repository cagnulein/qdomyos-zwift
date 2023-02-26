#pragma once

#include "Devices/bluetoothdevicetestdata.h"

class BikeTestData : public BluetoothDeviceTestData {
protected:
    void configureLockscreenSettings(const DeviceDiscoveryInfo& info,
                                            std::vector<LockscreenFunctionsTestData>& configurations) const override;

public:
    BikeTestData(std::string testName) : BluetoothDeviceTestData(testName) {}



};



