#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"
#include "devices/horizontreadmill/horizontreadmill.h"


class HorizonTreadmillTestData : public TreadmillTestData {

  public:
    HorizonTreadmillTestData() : TreadmillTestData("Horizon Treadmill") {

        this->addDeviceName("HORIZON", comparison::StartsWithIgnoreCase);
        this->addDeviceName("AFG SPORT", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WLT2541", comparison::StartsWithIgnoreCase);

        // FTMS
        this->addDeviceName("T318_", comparison::StartsWithIgnoreCase);
        this->addDeviceName("T218_", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TRX3500", comparison::StartsWithIgnoreCase);
        this->addDeviceName("JFTMPARAGON", comparison::StartsWithIgnoreCase);        
        this->addDeviceName("PARAGON X", comparison::StartsWithIgnoreCase); 
        this->addDeviceName("JFTM", comparison::StartsWithIgnoreCase);
        this->addDeviceName("CT800", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TRX4500", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MOBVOI TM", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ESANGLINKER", comparison::StartsWithIgnoreCase);
        this->addDeviceName("DK202000725", comparison::StartsWithIgnoreCase);
        this->addDeviceName("CTM780102C6BB32D62", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MX-TM ", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MATRIXTF50", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MOBVOI TM", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KETTLER TREADMILL", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ASSAULTRUNNER", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
    }
};

class HorizonTreadmillToorxTestData : public TreadmillTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            config.toorx_ftms_treadmill = true;
            config.toorx_ftms = false;
            configurations.push_back(config);
        } else {
            // Basic case where the device is disabled in the settings
            config.toorx_ftms_treadmill = false;
            config.toorx_ftms = false;
            configurations.push_back(config);

            // Basic case where the device is disabled in the settings and has an excluding settings
            config.toorx_ftms_treadmill = false;
            config.toorx_ftms = true;
            configurations.push_back(config);

            // Enabled in settings, but with excluding setting
            config.toorx_ftms_treadmill = true;
            config.toorx_ftms = true;
            configurations.push_back(config);
        }
    }

  public:
    HorizonTreadmillToorxTestData() : TreadmillTestData("Horizon Treadmill (Toorx)") {
        this->addDeviceName("TOORX", comparison::StartsWithIgnoreCase);
        this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
    }
};

class HorizonTreadmillBodyToneTestData : public TreadmillTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            config.horizon_treadmill_force_ftms = true;
            configurations.push_back(config);
        } else {
            // Basic case where the device is disabled in the settings
            config.horizon_treadmill_force_ftms = false;
            configurations.push_back(config);
        }
    }

  public:
    HorizonTreadmillBodyToneTestData() : TreadmillTestData("Horizon Treadmill (Bodytone)") {
        this->addDeviceName("TF-", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
    }
};


class HorizonTreadmillDomyosTCTestData : public TreadmillTestData {
protected:
    void configureBluetoothDeviceInfos(const QBluetoothDeviceInfo& info,  bool enable, std::vector<QBluetoothDeviceInfo>& bluetoothDeviceInfos) const override {
        auto result = info;
        if(enable) {
            result.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1826)}));
        }

        bluetoothDeviceInfos.push_back(result);
    }
public:
    HorizonTreadmillDomyosTCTestData() : TreadmillTestData("Horizon Treadmill (Domyos TC)") {
        this->addDeviceName("DOMYOS-TC", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<horizontreadmill *>(detectedDevice) != nullptr;
    }   
};
