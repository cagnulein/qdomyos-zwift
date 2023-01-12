#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "nordictrackelliptical.h"

class NordicTrackEllipticalTestData : public BluetoothDeviceTestData {

public:
    NordicTrackEllipticalTestData() : BluetoothDeviceTestData("Nordictrack Elliptical") {
        this->addDeviceName("I_EL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NordicTrackElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nordictrackelliptical*>(detectedDevice)!=nullptr;
    }
};

