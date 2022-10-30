#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "trxappgateusbtreadmill.h"

class TrxAppGateUSBTreadmillTestData : public BluetoothDeviceTestData {

public:
    TrxAppGateUSBTreadmillTestData();

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.toorx_bike = !enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbtreadmill*>(detectedDevice)!=nullptr;
    }
};

