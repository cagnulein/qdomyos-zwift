#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/DomyosElliptical/domyosellipticaltestdata.h"
#include "Devices/DomyosRower/domyosrowertestdata.h"
#include "Devices/DomyosBike/domyosbiketestdata.h"
#include "domyostreadmill.h"

class DomyosTreadmillTestData : public BluetoothDeviceTestData {

public:
    DomyosTreadmillTestData() {
        this->exclude(new DomyosEllipticalTestData());
        this->exclude(new DomyosBikeTestData());
        this->exclude(new DomyosRowerTestData());

        this->addDeviceName("Domyos", false, true);

        this->addInvalidDeviceName("DomyosBr");
        this->addInvalidDeviceName("DomyosBrX");
    }

    deviceType get_expectedDeviceType() const override { return deviceType::DomyosTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyostreadmill*>(detectedDevice)!=nullptr;
    }
};

