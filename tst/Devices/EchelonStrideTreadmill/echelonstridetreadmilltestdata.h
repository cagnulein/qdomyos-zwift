#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"


class EchelonStrideTreadmillTestData : public TreadmillTestData {

public:
    EchelonStrideTreadmillTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

