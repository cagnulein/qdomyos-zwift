#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "npecablebike.h"

class NPECableBikeTestData : public BluetoothDeviceTestData {
protected:
    NPECableBikeTestData() {}
public:
    deviceType get_expectedDeviceType() const override { return deviceType::NPECableBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<npecablebike*>(detectedDevice)!=nullptr;
    }
};

class NPECableBike1TestData : public NPECableBikeTestData {

public:
    NPECableBike1TestData() {
        this->addDeviceName(">CABLE", comparison::StartsWithIgnoreCase);
        this->addDeviceName("MD", comparison::StartsWithIgnoreCase, 7);
    }

};

class NPECableBike2TestData : public NPECableBikeTestData {

public:
    NPECableBike2TestData() {

        // BIKE 1, BIKE 2, BIKE 3...
        this->addDeviceName("BIKE", comparison::StartsWithIgnoreCase, 6);

        // put in a name that's too long.
        this->addInvalidDeviceName("BIKE567", comparison::IgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        // don't allow this device if the Flywheel bike is enabled.
        info.flywheel_life_fitness_ic8 = !enable;
    }
};
