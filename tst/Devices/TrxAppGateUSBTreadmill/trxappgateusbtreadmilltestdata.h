#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "trxappgateusbtreadmill.h"

class TrxAppGateUSBTreadmillTestData : public BluetoothDeviceTestData {

public:
    TrxAppGateUSBTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbtreadmill*>(detectedDevice)!=nullptr;
    }
};

