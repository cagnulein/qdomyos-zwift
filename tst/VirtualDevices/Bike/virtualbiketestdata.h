#pragma once

#include "VirtualDevices/virtualdevicetestdata.h"

class VirtualBikeTestData : public VirtualDeviceTestData {
public:
    VirtualBikeTestData();

    QZLockscreenFunctions::configurationType get_expectedLockscreenConfigurationType() const override;
    virtualdevice * createDevice(bluetoothdevice * device) override;
};

