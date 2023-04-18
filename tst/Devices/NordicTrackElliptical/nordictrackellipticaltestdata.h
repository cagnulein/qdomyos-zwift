#pragma once
#include "Devices/Elliptical/ellipticaltestdata.h"
#include "Devices/bluetoothdevicetestdata.h"
#include "nordictrackelliptical.h"

class NordicTrackEllipticalTestData : public EllipticalTestData {

public:
    NordicTrackEllipticalTestData() : EllipticalTestData("Nordictrack Elliptical") {
        this->addDeviceName("I_EL", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NordicTrackElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nordictrackelliptical*>(detectedDevice)!=nullptr;
    }
};

