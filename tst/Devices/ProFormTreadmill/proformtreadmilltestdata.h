#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"
#include "devices/proformtreadmill/proformtreadmill.h"


class ProFormTreadmillTestData : public TreadmillTestData {

public:
    ProFormTreadmillTestData(): TreadmillTestData("ProForm Treadmill") {
        this->addDeviceName("I_TL", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformtreadmill*>(detectedDevice)!=nullptr;
    }
};

