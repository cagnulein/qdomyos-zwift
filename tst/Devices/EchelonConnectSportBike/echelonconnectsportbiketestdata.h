#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/EchelonRower/echelonrowertestdata.h"
#include "Devices/EchelonStrideTreadmill/echelonstridetreadmilltestdata.h"
#include "devices/echelonconnectsport/echelonconnectsport.h"

class EchelonConnectSportBikeTestData : public BluetoothDeviceTestData {

public:
    EchelonConnectSportBikeTestData() : BluetoothDeviceTestData("Echelon Connect Sport Bike") {
        this->addDeviceName("ECH", comparison::StartsWith);
    }

    void configureExclusions() override {
        this->exclude(new EchelonRowerTestData());
        this->exclude(new EchelonStrideTreadmillTestData());
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonConnectSport; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonconnectsport*>(detectedDevice)!=nullptr;
    }
};

