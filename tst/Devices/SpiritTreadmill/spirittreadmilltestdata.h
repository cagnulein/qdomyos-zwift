#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"

#include "devices/spirittreadmill/spirittreadmill.h"


class SpiritTreadmillTestData : public TreadmillTestData {

public:
    SpiritTreadmillTestData(): TreadmillTestData("Spirit Treadmill") {

        this->addDeviceName("XT385", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XT485", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XT800", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XT900", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::SpiritTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<spirittreadmill*>(detectedDevice)!=nullptr;
    }
};

