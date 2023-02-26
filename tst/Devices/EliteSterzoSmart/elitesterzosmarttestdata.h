#pragma once

#include "Devices/Bike/biketestdata.h"


class EliteSterzoSmartTestData : public BikeTestData {

public:
    EliteSterzoSmartTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

