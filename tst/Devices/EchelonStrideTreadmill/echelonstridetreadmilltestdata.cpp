#include "echelonstridetreadmilltestdata.h" 
#include "echelonstride.h"

EchelonStrideTreadmillTestData::EchelonStrideTreadmillTestData() : TreadmillTestData("Echelon Stride Treadmill") {
    this->addDeviceName("ECH-STRIDE", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ECH-UK-", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ECH-SD-SPT", comparison::StartsWithIgnoreCase);
}

deviceType EchelonStrideTreadmillTestData::get_expectedDeviceType() const { return deviceType::EchelonStride; }

bool EchelonStrideTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<echelonstride*>(detectedDevice)!=nullptr;
}
