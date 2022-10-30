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

        this->addDeviceName("FS-", comparison::StartsWith);
        this->exclude(new FTMSBikeTestData());
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
        // SW + 12 characters
        this->addDeviceName("SW0123456789ABCD", comparison::Exact);
        this->addDeviceName("SWFOURTEENCHARS.", comparison::Exact);

        this->addInvalidDeviceName("SW0123456789ABC", comparison::Exact);
        this->addInvalidDeviceName("SW0123456789ABCDE", comparison::Exact);

    }
};

class FitshowTreadmillBFTestData : public FitshowTreadmillTestData {
public:
    FitshowTreadmillBFTestData() {
        this->addDeviceName("BF70", comparison::StartsWith);
    }
};
