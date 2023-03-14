#pragma once

#include "VirtualDevices/virtualdevicetestdata.h"

class VirtualTreadmillTestData : public VirtualDeviceTestData {
public:
    VirtualTreadmillTestData();

    QZLockscreenFunctions::configurationType get_expectedLockscreenConfigurationType() const override  {
        return QZLockscreenFunctions::configurationType::TREADMILL;
    }

    virtualdevice * createDevice(bluetoothdevice * device) override;
};
