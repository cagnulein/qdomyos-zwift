#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "trxappgateusbbike.h"

class TrxAppGateUSBBikeTestData : public BluetoothDeviceTestData {

public:
    TrxAppGateUSBBikeTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbbike*>(detectedDevice)!=nullptr;
    }
};

