#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "toorxtreadmill.h"

class ToorxTreadmillTestData : public TreadmillTestData {

public:
    ToorxTreadmillTestData() : TreadmillTestData("Toorx Treadmill") {
        this->addDeviceName("TRX ROUTE KEY", comparison::StartsWith);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::ToorxTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<toorxtreadmill*>(detectedDevice)!=nullptr;
    }
};

