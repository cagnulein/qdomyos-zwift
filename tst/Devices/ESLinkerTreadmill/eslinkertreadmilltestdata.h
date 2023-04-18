#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "eslinkertreadmill.h"

class ESLinkerTreadmillTestData : public TreadmillTestData {

public:
    ESLinkerTreadmillTestData(): TreadmillTestData("ES Linker Treadmill") {
        this->addDeviceName("ESLINKER", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ESLinkerTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<eslinkertreadmill*>(detectedDevice)!=nullptr;
    }
};

