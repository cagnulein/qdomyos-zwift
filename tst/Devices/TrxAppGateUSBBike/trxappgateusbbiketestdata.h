#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.h"
#include "trxappgateusbbike.h"

class TrxAppGateUSBBikeTestData : public BluetoothDeviceTestData {
protected:
    TrxAppGateUSBBikeTestData() {

    }

    void configureExclusions() override {
        this->exclude(new TrxAppGateUSBTreadmillTestData);
    }
public:
    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbbike*>(detectedDevice)!=nullptr;
    }
};

class TrxAppGateUSBBike1TestData : public TrxAppGateUSBBikeTestData {

public:
    TrxAppGateUSBBike1TestData()  {
        this->addDeviceName("TUN ", comparison::StartsWithIgnoreCase);

    }

};

class TrxAppGateUSBBike2TestData : public TrxAppGateUSBBikeTestData {

public:
    TrxAppGateUSBBike2TestData() {
        this->hasSettings = true;

        this->addDeviceName("TOORX", comparison::StartsWith);
        this->addDeviceName("I-CONSOIE+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("IBIKING+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("ICONSOLE+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("VIFHTR2.1", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("DKN MOTION", comparison::StartsWithIgnoreCase);
        this->addDeviceName("CR011R", comparison::IgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        info.toorx_bike = enable;
    }
};
