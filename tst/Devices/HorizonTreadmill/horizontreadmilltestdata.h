#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "horizontreadmill.h"

class HorizonTreadmillTestData : public BluetoothDeviceTestData {

public:
    HorizonTreadmillTestData() {

        this->addDeviceName("HORIZON", comparison::StartsWithIgnoreCase);
        this->addDeviceName("AFG SPORT", comparison::StartsWithIgnoreCase);
        this->addDeviceName("WLT2541", comparison::StartsWithIgnoreCase);
        this->addDeviceName("S77", comparison::StartsWithIgnoreCase);

        // FTMS
        this->addDeviceName("T318_", comparison::StartsWithIgnoreCase);
        this->addDeviceName("T218_", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TRX3500", comparison::StartsWithIgnoreCase);
        this->addDeviceName("JFTMPARAGON", comparison::StartsWithIgnoreCase);
        this->addDeviceName("JFTM", comparison::StartsWithIgnoreCase);
        this->addDeviceName("CT800", comparison::StartsWithIgnoreCase);
        this->addDeviceName("TRX4500", comparison::StartsWithIgnoreCase);
        this->addDeviceName("ESANGLINKER", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::HorizonTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<horizontreadmill*>(detectedDevice)!=nullptr;
    }
};

