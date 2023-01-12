#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "iconceptbike.h"

class iConceptBikeTestData : public BluetoothDeviceTestData {

public:
    iConceptBikeTestData() : BluetoothDeviceTestData("iConcept Bike") {
        this->addDeviceName("BH DUALKIT", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::IConceptBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<iconceptbike*>(detectedDevice)!=nullptr;
    }
};

