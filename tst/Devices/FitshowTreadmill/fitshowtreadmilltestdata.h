#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fitshowtreadmill.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

class FitshowTreadmillTestData : public BluetoothDeviceTestData {

protected:

    FitshowTreadmillTestData(std::string testName) : BluetoothDeviceTestData(testName) {

    }

public:

    deviceType get_expectedDeviceType() const override { return deviceType::FitshowTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitshowtreadmill*>(detectedDevice)!=nullptr;
    }
};


class FitshowTreadmillFSTestData : public FitshowTreadmillTestData {
protected:
    void configureSettings(const DeviceDiscoveryInfo& info, bool enable, std::vector<DeviceDiscoveryInfo> configurations) const override {
        DeviceDiscoveryInfo config(info);

        if(enable){
            config.snode_bike = false;
            config.fitplus_bike = false;
            configurations.push_back(config);
        } else {
            for(int i=1; i<4; i++) {
                config.snode_bike = i&1;
                config.fitplus_bike = i&2;
                configurations.push_back(config);
            }
        }
    }
public:
    FitshowTreadmillFSTestData() : FitshowTreadmillTestData("FitShow FS") {
        this->addDeviceName("FS-", comparison::StartsWith);
    }

    void configureExclusions() override {
        this->exclude(new FTMSBike1TestData());
        this->exclude(new FTMSBike2TestData());
    }
};


class FitshowTreadmillSWTestData : public FitshowTreadmillTestData {
public:
    FitshowTreadmillSWTestData(): FitshowTreadmillTestData("FitShow SW") {
        // SW, 14 characters total
        this->addDeviceName("SW345678901234", comparison::Exact);
        this->addDeviceName("SWFOURTEENCHAR", comparison::Exact);
        this->addDeviceName("WINFITA", comparison::StartsWithIgnoreCase);

        // too long and too short
        this->addInvalidDeviceName("SW3456789012345", comparison::Exact);
        this->addInvalidDeviceName("SW34567890123", comparison::Exact);

    }
};

class FitshowTreadmillBFTestData : public FitshowTreadmillTestData {
public:
    FitshowTreadmillBFTestData() : FitshowTreadmillTestData("FitShow BF"){
        this->addDeviceName("BF70", comparison::StartsWith);
    }
};
