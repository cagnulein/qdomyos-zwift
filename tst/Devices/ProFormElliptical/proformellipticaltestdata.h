#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformelliptical.h"

class ProFormEllipticalTestData : public BluetoothDeviceTestData {

public:
    ProFormEllipticalTestData() {
        this->addDeviceName("I_FS", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::ProformElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformelliptical*>(detectedDevice)!=nullptr;
    }
};

