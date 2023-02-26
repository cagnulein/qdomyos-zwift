#include "proformellipticaltestdata.h" 
#include "proformelliptical.h"

ProFormEllipticalTestData::ProFormEllipticalTestData() : EllipticalTestData("ProForm Elliptical") {
    this->addDeviceName("I_FS", comparison::StartsWithIgnoreCase);
}

deviceType ProFormEllipticalTestData::get_expectedDeviceType() const { return deviceType::ProformElliptical; }

bool ProFormEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<proformelliptical*>(detectedDevice)!=nullptr;
}
