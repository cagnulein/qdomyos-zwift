#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "Devices/DomyosElliptical/domyosellipticaltestdata.h"
#include "Devices/DomyosRower/domyosrowertestdata.h"
#include "Devices/DomyosBike/domyosbiketestdata.h"
#include "domyostreadmill.h"

class DomyosTreadmillTestData : public TreadmillTestData {

public:
    DomyosTreadmillTestData() : TreadmillTestData("Domyos Treadmill") {
        this->addDeviceName("Domyos", comparison::StartsWith);

        this->addInvalidDeviceName("DomyosBr", comparison::StartsWith);
    }

    void configureExclusions() override {
        this->exclude(new DomyosEllipticalTestData());
        this->exclude(new DomyosBikeTestData());
        this->exclude(new DomyosRowerTestData());
    }

    deviceType get_expectedDeviceType() const override { return deviceType::DomyosTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<domyostreadmill*>(detectedDevice)!=nullptr;
    }
};

