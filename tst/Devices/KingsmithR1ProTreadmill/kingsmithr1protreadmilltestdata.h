#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/KingsmithR2Treadmill/kingsmithr2treadmilltestdata.h"
#include "kingsmithr1protreadmill.h"

class KingsmithR1ProTreadmillTestData : public BluetoothDeviceTestData {
protected:
    void configureExclusions() override {
        this->exclude(new KingsmithR2TreadmillTestData());
    }
public:
    KingsmithR1ProTreadmillTestData() : BluetoothDeviceTestData("Kingsmith R1 Pro Treadmill") {

        this->addDeviceName("R1 PRO", comparison::StartsWithIgnoreCase);
        this->addDeviceName("RE", comparison::IgnoreCase);
        this->addDeviceName("KINGSMITH", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-H", comparison::StartsWithIgnoreCase);
        this->addDeviceName("DYNAMAX", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WALKINGPAD", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::KingsmithR1ProTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<kingsmithr1protreadmill*>(detectedDevice)!=nullptr;
    }
};

