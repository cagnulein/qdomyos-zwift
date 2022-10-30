#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "echelonconnectsport.h"

class EchelonConnectSportBikeTestData : public BluetoothDeviceTestData {

public:
    EchelonConnectSportBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonConnectSport; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonconnectsport*>(detectedDevice)!=nullptr;
    }
};

