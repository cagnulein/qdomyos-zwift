#pragma once
#include "Devices/Elliptical/ellipticaltestdata.h"
#include "devices/proformellipticaltrainer/proformellipticaltrainer.h"

class ProFormEllipticalTrainerTestData : public EllipticalTestData {

public:
    ProFormEllipticalTrainerTestData(): EllipticalTestData("ProForm Elliptical Trainer") {
        this->addDeviceName("I_VE", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformEllipticalTrainer; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformellipticaltrainer*>(detectedDevice)!=nullptr;
    }
};

