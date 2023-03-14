#include "bhfitnessellipticaltestdata.h"
#include "bhfitnesselliptical.h"

BHFitnessEllipticalTestData::BHFitnessEllipticalTestData() : EllipticalTestData("BH Fitness Elliptical") {
    this->addDeviceName("B01_", comparison::StartsWithIgnoreCase);
}

deviceType BHFitnessEllipticalTestData::get_expectedDeviceType() const { return deviceType::BHFitnessElliptical; }

bool BHFitnessEllipticalTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<bhfitnesselliptical*>(detectedDevice)!=nullptr;
}
