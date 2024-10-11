#pragma once


#include "Devices/Rower/rowertestdata.h"

#include "devices/echelonrower/echelonrower.h"


class EchelonRowerTestData : public RowerTestData {

public:
    EchelonRowerTestData() : RowerTestData("Echelon Rower") {
        this->addDeviceName("ECH-ROW", comparison::StartsWith);
        this->addDeviceName("ROWSPORT-", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ROW-S", comparison::StartsWith);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonRower; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonrower*>(detectedDevice)!=nullptr;
    }
};

