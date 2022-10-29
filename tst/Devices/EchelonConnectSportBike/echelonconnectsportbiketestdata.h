#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "echelonconnectsportbike.h"

class EchelonConnectSportBikeTestData : public BluetoothDeviceTestData {

public:
    EchelonConnectSportBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::EchelonConnectSportBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<echelonconnectsportbike*>(detectedDevice)!=nullptr;
    }
};

