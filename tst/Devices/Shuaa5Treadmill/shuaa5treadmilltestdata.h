#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"

#include "devices/shuaa5treadmill/shuaa5treadmill.h"


class Shuaa5TreadmillTestData : public TreadmillTestData {

public:
    Shuaa5TreadmillTestData() : TreadmillTestData("Shuaa5 Treadmill") {
        this->addDeviceName("ZW-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::Shuaa5Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<shuaa5treadmill*>(detectedDevice)!=nullptr;
    }
};

