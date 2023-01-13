#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "proformellipticaltrainer.h"

class ProFormEllipticalTrainerTestData : public BluetoothDeviceTestData {

public:
    ProFormEllipticalTrainerTestData(): BluetoothDeviceTestData("ProForm Elliptical Trainer") {
        this->addDeviceName("I_VE", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformEllipticalTrainer; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformellipticaltrainer*>(detectedDevice)!=nullptr;
    }
};

