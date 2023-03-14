#include "proformellipticaltrainertestdata.h" 
#include "proformellipticaltrainer.h"

ProFormEllipticalTrainerTestData::ProFormEllipticalTrainerTestData(): EllipticalTestData("ProForm Elliptical Trainer") {
    this->addDeviceName("I_VE", comparison::StartsWithIgnoreCase);
}

deviceType ProFormEllipticalTrainerTestData::get_expectedDeviceType() const { return deviceType::ProformEllipticalTrainer; }

bool ProFormEllipticalTrainerTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<proformellipticaltrainer*>(detectedDevice)!=nullptr;
}
