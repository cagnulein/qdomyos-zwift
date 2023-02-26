#include "eslinkertreadmilltestdata.h" 
#include "eslinkertreadmill.h"

ESLinkerTreadmillTestData::ESLinkerTreadmillTestData(): TreadmillTestData("ES Linker Treadmill") {
    this->addDeviceName("ESLINKER", comparison::StartsWithIgnoreCase);
}

deviceType ESLinkerTreadmillTestData::get_expectedDeviceType() const { return deviceType::ESLinkerTreadmill; }

bool ESLinkerTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<eslinkertreadmill*>(detectedDevice)!=nullptr;
}
