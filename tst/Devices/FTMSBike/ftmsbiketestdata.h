#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "ftmsbike.h"

#include "Devices/SnodeBike/snodebiketestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/StagesBike/stagesbiketestdata.h"

class FTMSBikeTestData : public BluetoothDeviceTestData {
protected:
    void configureExclusions() override;

    FTMSBikeTestData(std::string testName) : BluetoothDeviceTestData(testName)  {

    }
public:

    deviceType get_expectedDeviceType() const override { return deviceType::FTMSBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ftmsbike*>(detectedDevice)!=nullptr;
    }
};

class FTMSBike1TestData : public FTMSBikeTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.hammerRacerS = enable;
        return true;
    }
public:

    FTMSBike1TestData() : FTMSBikeTestData("FTMS Bike Hammer Racer S") {
        this->addDeviceName("FS-", comparison::StartsWith);
    }


};

class FTMSBike2TestData : public FTMSBikeTestData {
public:

    FTMSBike2TestData() : FTMSBikeTestData("FTMS Bike")  {
        this->addDeviceName("DHZ-", comparison::StartsWithIgnoreCase); // JK fitness 577
        this->addDeviceName("MKSM", comparison::StartsWithIgnoreCase); // MKSM3600036
        this->addDeviceName("YS_C1_", comparison::StartsWithIgnoreCase);// Yesoul C1H
        this->addDeviceName("DS25-", comparison::StartsWithIgnoreCase); // Bodytone DS25
        this->addDeviceName("SCHWINN 510T", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ZWIFT HUB", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MAGNUS ", comparison::StartsWithIgnoreCase); // Saris Trainer
        this->addDeviceName("FLXCY-", comparison::StartsWithIgnoreCase); // Pro FlexBike        
        this->addDeviceName("KICKR CORE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("B94", comparison::StartsWithIgnoreCase);
        this->addDeviceName("STAGES BIKE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("SUITO", comparison::StartsWithIgnoreCase);
        this->addDeviceName("D2RIDE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("DIRETO XR", comparison::StartsWithIgnoreCase);
        this->addDeviceName("SMB1", comparison::StartsWithIgnoreCase);
        this->addDeviceName("INRIDE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("XBR55", comparison::StartsWithIgnoreCase);
        this->addDeviceName("HAMMER ", comparison::StartsWithIgnoreCase); // HAMMER 64123


        // Starts with DT- and is 14+ characters long.
        // TODO: update once addDeviceName can generate valid and invalid names for variable length patterns
        this->addDeviceName("DT-0123456789A", comparison::IgnoreCase); // Sole SB700
        this->addDeviceName("DT-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", comparison::IgnoreCase); // Sole SB700
        this->addInvalidDeviceName("DT-0123456789", comparison::IgnoreCase); // too short for Sole SB700
     }
};

class FTMSBike3TestData : public FTMSBikeTestData {
private:
    QString ftmsAccessoryName;
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.ss2k_peloton = enable;

        if(enable)
            info.ftmsAccessoryName = this->ftmsAccessoryName;
        else
            info.ftmsAccessoryName = "NOT "+this->ftmsAccessoryName;

        return true;
    }
public:
    FTMSBike3TestData() : FTMSBikeTestData("FTMS Accessory") {
        this->ftmsAccessoryName = "accessory";

        this->addDeviceName(this->ftmsAccessoryName, comparison::StartsWithIgnoreCase);
    }
};
