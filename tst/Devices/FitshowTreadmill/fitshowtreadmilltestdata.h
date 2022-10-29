#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "fitshowtreadmill.h"

class FitshowTreadmillTestData : public BluetoothDeviceTestData {

public:
    FitshowTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FitshowTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fitshowtreadmill*>(detectedDevice)!=nullptr;
    }
};

