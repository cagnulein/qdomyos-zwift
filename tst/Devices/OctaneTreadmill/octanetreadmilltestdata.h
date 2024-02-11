#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"


#include "devices/octanetreadmill/octanetreadmill.h"


class OctaneTreadmillTestData : public TreadmillTestData {

public:
    OctaneTreadmillTestData() : TreadmillTestData("Octane Treadmill") {
        this->addDeviceName("ZR7", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ZR8", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::OctaneTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<octanetreadmill*>(detectedDevice)!=nullptr;
    }
};

