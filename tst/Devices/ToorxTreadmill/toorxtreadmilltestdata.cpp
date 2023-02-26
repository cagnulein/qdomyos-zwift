#include "toorxtreadmilltestdata.h" 
#include "toorxtreadmill.h"

ToorxTreadmillTestData::ToorxTreadmillTestData() : TreadmillTestData("Toorx Treadmill") {
    this->addDeviceName("TRX ROUTE KEY", comparison::StartsWith);
}

deviceType ToorxTreadmillTestData::get_expectedDeviceType() const { return deviceType::ToorxTreadmill; }

bool ToorxTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<toorxtreadmill*>(detectedDevice)!=nullptr;
}
