#pragma once


#include "Devices/Elliptical/ellipticaltestdata.h"
#include "devices/trxappgateusbelliptical/trxappgateusbelliptical.h"


class TrxAppGateUSBEllipticalTestData : public EllipticalTestData {

public:
    TrxAppGateUSBEllipticalTestData() : EllipticalTestData("TrxAppGateUSB Elliptical") {
        DeviceDiscoveryInfo config(info);
        this->addDeviceName("FAL-SPORTS", comparison::StartsWithIgnoreCase);
        if(enable) {
            config.ftms_bike = "Disabled";
            configurations.push_back(config);
        } else {
            config.ftms_bike = "PLACEHOLDER";
            configurations.push_back(config);
        }        
    }


    deviceType get_expectedDeviceType() const override { return deviceType::TrxAppGateUSBElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trxappgateusbelliptical*>(detectedDevice)!=nullptr;
    }
};
