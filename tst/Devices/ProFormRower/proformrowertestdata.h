#pragma once

#include "Devices/Rower/rowertestdata.h"
#include "devices/proformrower/proformrower.h"

class ProFormRowerTestData : public RowerTestData {

public:
    ProFormRowerTestData() : RowerTestData("ProForm Rower") {
        this->addDeviceName("I_RW", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformrower*>(detectedDevice)!=nullptr;
    }
};

