#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"


#include "devices/echelonstride/echelonstride.h"


class EchelonStrideTreadmillTestData : public TreadmillTestData {

public:
    EchelonStrideTreadmillTestData() : TreadmillTestData("Echelon Stride Treadmill") {
        this->addDeviceName("ECH-STRIDE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ECH-UK-", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ECH-SD-SPT", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonStride; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonstride*>(detectedDevice)!=nullptr;
    }
};

