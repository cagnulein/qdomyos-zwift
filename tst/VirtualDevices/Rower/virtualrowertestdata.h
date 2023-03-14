#pragma once

#include "VirtualDevices/virtualdevicetestdata.h"

class VirtualRowerTestData : public VirtualDeviceTestData {
public:
    VirtualRowerTestData();

    QZLockscreenFunctions::configurationType get_expectedLockscreenConfigurationType() const override  {
        return QZLockscreenFunctions::configurationType::ROWER;
    }

    virtualdevice * createDevice(bluetoothdevice * device) override;
};
