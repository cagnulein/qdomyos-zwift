#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "nordictrackifitadbtreadmill.h"

class NordicTrackIFitADBTreadmillTestData : public BluetoothDeviceTestData {

public:
    NordicTrackIFitADBTreadmillTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NordicTrackIFitADBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nordictrackifitadbtreadmill*>(detectedDevice)!=nullptr;
    }
};

