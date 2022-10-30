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
public:
    FitshowTreadmillFSTestData() {
        this->hasSettings = true;
        this->addDeviceName("FS-", comparison::StartsWith);
    }

    void configureExclusions() override {
        this->exclude(new FTMSBike1TestData());
        this->exclude(new FTMSBike2TestData());
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        // TODO test if only 1 of these is set
        info.snode_bike = !enable;
        info.fitplus_bike = !enable;
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
