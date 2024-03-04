#pragma once


#include "Devices/Elliptical/ellipticaltestdata.h"
#include "devices/ypooelliptical/ypooelliptical.h"


class YpooEllipticalTestData : public EllipticalTestData {

public:
    YpooEllipticalTestData() : EllipticalTestData("Ypoo Elliptical") {
        this->addDeviceName("YPOO-U3-", comparison::StartsWithIgnoreCase);
        this->addDeviceName("SCH_590E", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::YpooElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<ypooelliptical*>(detectedDevice)!=nullptr;
    }
};
