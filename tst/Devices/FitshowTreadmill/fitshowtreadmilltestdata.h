#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fitshowtreadmill.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

class FitshowTreadmillTestData : public BluetoothDeviceTestData {

protected:

    FitshowTreadmillTestData() {

    }

public:

    deviceType get_expectedDeviceType() const override { return deviceType::FitshowTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitshowtreadmill*>(detectedDevice)!=nullptr;
    }
};


class FitshowTreadmillFSTestData : public FitshowTreadmillTestData {
protected:
    void configureSettings(const devicediscoveryinfo& info, bool enable, std::vector<devicediscoveryinfo> configurations) const override {
        if(enable){
            devicediscoveryinfo info1(info);
            info1.snode_bike = false;
            info1.fitplus_bike = false;
            configurations.push_back(info1);
        } else {
            devicediscoveryinfo info1(info);
            info1.snode_bike = true;
            info1.fitplus_bike = true;
            configurations.push_back(info1);

            devicediscoveryinfo info2(info);
            info1.snode_bike = true;
            info1.fitplus_bike = false;
            configurations.push_back(info2);

            devicediscoveryinfo info3(info);
            info1.snode_bike = false;
            info1.fitplus_bike = true;
            configurations.push_back(info3);
        }
    }
public:
    FitshowTreadmillFSTestData() {
        this->addDeviceName("FS-", comparison::StartsWith);
    }

    void configureExclusions() override {
        this->exclude(new FTMSBike1TestData());
        this->exclude(new FTMSBike2TestData());
    }


};


class FitshowTreadmillSWTestData : public FitshowTreadmillTestData {
public:
    FitshowTreadmillSWTestData() {
        // SW, 14 characters total
        this->addDeviceName("SW345678901234", comparison::Exact);
        this->addDeviceName("SWFOURTEENCHAR", comparison::Exact);

        // too long and too short
        this->addInvalidDeviceName("SW3456789012345", comparison::Exact);
        this->addInvalidDeviceName("SW34567890123", comparison::Exact);

    }
};

class FitshowTreadmillBFTestData : public FitshowTreadmillTestData {
public:
    FitshowTreadmillBFTestData() {
        this->addDeviceName("BF70", comparison::StartsWith);
    }
};
