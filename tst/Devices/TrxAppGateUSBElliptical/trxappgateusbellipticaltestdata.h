#pragma once


#include "Devices/Elliptical/ellipticaltestdata.h"
#include "devices/trxappgateusbelliptical/trxappgateusbelliptical.h"


class TrxAppGateUSBEllipticalTestData : public EllipticalTestData {

public:
    TrxAppGateUSBEllipticalTestData() : EllipticalTestData("TrxAppGateUSB Elliptical") {
        this->addDeviceName("FAL-SPORTS", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbelliptical*>(detectedDevice)!=nullptr;
    }
};
