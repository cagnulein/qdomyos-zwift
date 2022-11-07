#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "concept2skierg.h"

class Concept2SkiErgTestData : public BluetoothDeviceTestData {

public:
    Concept2SkiErgTestData() : BluetoothDeviceTestData("Concept2 Ski Erg") {
        this->addDeviceName("PM5", "SKI", comparison::IgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::Concept2SkiErg; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<concept2skierg*>(detectedDevice)!=nullptr;
    }
};

