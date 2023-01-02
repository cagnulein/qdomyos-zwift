#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.h"
#include "trxappgateusbbike.h"

class TrxAppGateUSBBikeTestData : public BluetoothDeviceTestData {
protected:
    TrxAppGateUSBBikeTestData(std::string testName) : BluetoothDeviceTestData(testName) {

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
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo> configurations) const override {
        // This particular case of TrxAppGateUSBBike is independant of the setting

        DeviceDiscoveryInfo info1(info);
        info1.toorx_bike = true;
        configurations.push_back(info1);

        DeviceDiscoveryInfo info2(info);
        info2.toorx_bike = false;
        configurations.push_back(info2);
    }
public:
    TrxAppGateUSBBike1TestData() : TrxAppGateUSBBikeTestData("Toorx AppGate USB Bike")  {
        this->addDeviceName("TUN ", comparison::StartsWithIgnoreCase);
    }

};

class TrxAppGateUSBBike2TestData : public TrxAppGateUSBBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.toorx_bike = enable;
        return true;
    }
public:
    TrxAppGateUSBBike2TestData() : TrxAppGateUSBBikeTestData("Toorx AppGate USB Bike (enabled in settings)") {

        this->addDeviceName("TOORX", comparison::StartsWith);
        this->addDeviceName("I-CONSOIE+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("IBIKING+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("ICONSOLE+", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("VIFHTR2.1", comparison::StartsWithIgnoreCase) ;
        this->addDeviceName("DKN MOTION", comparison::StartsWithIgnoreCase);
        this->addDeviceName("CR011R", comparison::IgnoreCase);
    }
};
