#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "Devices/KingsmithR2Treadmill/kingsmithr2treadmilltestdata.h"
#include "kingsmithr1protreadmill.h"

class KingsmithR1ProTreadmillTestData : public BluetoothDeviceTestData {

public:
    KingsmithR1ProTreadmillTestData() {

        this->addDeviceName("R1 PRO", comparison::StartsWithIgnoreCase);
        this->addDeviceName("RE", comparison::IgnoreCase);
        this->addDeviceName("KINGSMITH", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-", comparison::StartsWithIgnoreCase);

        this->exclude(new KingsmithR2TreadmillTestData());
    }

    deviceType get_expectedDeviceType() const override { return deviceType::KingsmithR1ProTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<kingsmithr1protreadmill*>(detectedDevice)!=nullptr;
    }
};

