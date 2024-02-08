#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "devices/proformelliptical/proformelliptical.h"

class ProFormEllipticalTestData : public BluetoothDeviceTestData {

public:
    ProFormEllipticalTestData() : BluetoothDeviceTestData("ProForm Elliptical") {
        this->addDeviceName("I_FS", comparison::StartsWithIgnoreCase);
    }


    deviceType get_expectedDeviceType() const override { return deviceType::ProformElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformelliptical*>(detectedDevice)!=nullptr;
    }
};

