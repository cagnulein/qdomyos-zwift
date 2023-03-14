#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class BowflexTreadmillTestData : public TreadmillTestData {

public:
    BowflexTreadmillTestData();

    QStringList get_deviceNames() const override;

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

