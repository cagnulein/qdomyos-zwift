#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/TrxAppGateUSBBike/trxappgateusbbiketestdata.h"
#include "trxappgateusbtreadmill.h"

class TrxAppGateUSBTreadmillTestData : public BluetoothDeviceTestData {

public:
    TrxAppGateUSBTreadmillTestData() {

        this->addDeviceName("TOORX", comparison::StartsWith);
        this->addDeviceName("V-RUN", comparison::StartsWith);

        this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ICONSOLE+", comparison::StartsWithIgnoreCase);
        this->addDeviceName("I-RUNNING", comparison::StartsWithIgnoreCase);
        this->addDeviceName("DKN RUN", comparison::StartsWithIgnoreCase);
        this->addDeviceName("REEBOK", comparison::StartsWithIgnoreCase);


        this->exclude(new TrxAppGateUSBBike1TestData());
        this->exclude(new TrxAppGateUSBBike2TestData());
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.toorx_bike = !enable;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbtreadmill*>(detectedDevice)!=nullptr;
    }
};

