#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "nordictrackelliptical.h"

class NordicTrackEllipticalTestData : public BluetoothDeviceTestData {

public:
    NordicTrackEllipticalTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NordicTrackElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nordictrackelliptical*>(detectedDevice)!=nullptr;
    }
};

