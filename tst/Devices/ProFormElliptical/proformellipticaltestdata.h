#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformelliptical.h"

class ProFormEllipticalTestData : public BluetoothDeviceTestData {

public:
    ProFormEllipticalTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformelliptical*>(detectedDevice)!=nullptr;
    }
};

