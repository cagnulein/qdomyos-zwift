#pragma once

#include "Devices/Bike/biketestdata.h"
#include "Devices/TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.h"
#include "devices/trxappgateusbbike/trxappgateusbbike.h"

class TrxAppGateUSBBikeTestData : public BikeTestData {
protected:
    TrxAppGateUSBBikeTestData(std::string testName) : BikeTestData(testName) {

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
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        // This particular case of TrxAppGateUSBBike is independent of the setting

        DeviceDiscoveryInfo config(info);
        config.toorx_bike = true;
        config.toorx_ftms_treadmill = !enable;
        configurations.push_back(config);

        config.toorx_bike = false;
        config.toorx_ftms_treadmill = !enable;
        configurations.push_back(config);
    }
public:
    TrxAppGateUSBBike1TestData() : TrxAppGateUSBBikeTestData("Toorx AppGate USB Bike")  {
        this->addDeviceName("TUN ", comparison::StartsWithIgnoreCase);
        this->addDeviceName("PASYOU-", comparison::StartsWithIgnoreCase);
    }

};

class TrxAppGateUSBBike2TestData : public TrxAppGateUSBBikeTestData {
protected:

    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo>& configurations) const override {
        DeviceDiscoveryInfo config(info);

        if(enable) {
            config.toorx_bike = true;
            config.toorx_ftms_treadmill = false;
            configurations.push_back(config);
        } else {
            config.toorx_bike = false;
            config.toorx_ftms_treadmill = true;
            configurations.push_back(config);
            config.toorx_bike = false;
            config.toorx_ftms_treadmill = false;
            configurations.push_back(config);
        }
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
