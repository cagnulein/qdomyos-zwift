#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"

#include "devices/ziprotreadmill/ziprotreadmill.h"


class ZiproTreadmillTestData : public TreadmillTestData {

public:
    ZiproTreadmillTestData() : TreadmillTestData("Zipro Treadmill") {
        this->addDeviceName("RZ_TREADMIL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ZiproTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ziprotreadmill*>(detectedDevice)!=nullptr;
    }
};

