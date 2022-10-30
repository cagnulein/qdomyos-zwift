#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/EchelonRower/echelonrowertestdata.h"
#include "Devices/EchelonStrideTreadmill/echelonstridetreadmilltestdata.h"
#include "echelonconnectsport.h"

class EchelonConnectSportBikeTestData : public BluetoothDeviceTestData {

public:
    EchelonConnectSportBikeTestData() {
        this->addDeviceName("ECH", comparison::StartsWith);

        this->exclude(new EchelonRowerTestData());
        this->exclude(new EchelonStrideTreadmillTestData());
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonConnectSport; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonconnectsport*>(detectedDevice)!=nullptr;
    }
};

