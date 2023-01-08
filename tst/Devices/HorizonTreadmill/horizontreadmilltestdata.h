#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "horizontreadmill.h"

class HorizonTreadmillTestData : public BluetoothDeviceTestData {

public:
    HorizonTreadmillTestData() : BluetoothDeviceTestData("Horizon Treadmill") {

        this->addDeviceName("HORIZON", comparison::StartsWithIgnoreCase);
        this->addDeviceName("AFG SPORT", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WLT2541", comparison::StartsWithIgnoreCase);
        this->addDeviceName("S77", comparison::StartsWithIgnoreCase);

        // FTMS
        this->addDeviceName("T318_", comparison::StartsWithIgnoreCase);
        this->addDeviceName("T218_", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TRX3500", comparison::StartsWithIgnoreCase);
        this->addDeviceName("JFTMPARAGON", comparison::StartsWithIgnoreCase);
        this->addDeviceName("JFTM", comparison::StartsWithIgnoreCase);
        this->addDeviceName("CT800", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TRX4500", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MOBVOI TM", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ESANGLINKER", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<horizontreadmill*>(detectedDevice)!=nullptr;
    }
};

class HorizonTreadmillToorxTestData : public BluetoothDeviceTestData {
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo> configurations) const override {
        if(enable){
            DeviceDiscoveryInfo info1(info);
            info1.toorx_ftms_treadmill = true;
            info1.toorx_ftms = false;
            configurations.push_back(info1);
        } else {
            {
                // Basic case where the device is disabled in the settings
                DeviceDiscoveryInfo info1(info);
                info1.toorx_ftms_treadmill = false;
                info1.toorx_ftms = false;
                configurations.push_back(info1);
            }

            {
                // Basic case where the device is disabled in the settings and has an excluding settings
                DeviceDiscoveryInfo info1(info);
                info1.toorx_ftms_treadmill = false;
                info1.toorx_ftms = true;
                configurations.push_back(info1);
            }

            {
                // Enabled in settings, but with excluding setting
                DeviceDiscoveryInfo info1(info);
                info1.toorx_ftms_treadmill = true;
                info1.toorx_ftms = true;
                configurations.push_back(info1);
            }
        }
    }
public:
    HorizonTreadmillToorxTestData() : BluetoothDeviceTestData("Horizon Treadmill (Toorx)") {
        this->addDeviceName("TOORX", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<horizontreadmill*>(detectedDevice)!=nullptr;
    }
};
