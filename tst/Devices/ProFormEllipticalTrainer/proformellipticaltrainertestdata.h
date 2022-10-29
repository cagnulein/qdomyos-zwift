#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformellipticaltrainer.h"

class ProFormEllipticalTrainerTestData : public BluetoothDeviceTestData {

public:
    ProFormEllipticalTrainerTestData() {}

    QStringList get_deviceNames() const override {
        QStringList result;

        return result;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProFormEllipticalTrainer; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformellipticaltrainer*>(detectedDevice)!=nullptr;
    }
};

