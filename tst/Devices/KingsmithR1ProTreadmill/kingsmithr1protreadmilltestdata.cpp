#include "kingsmithr1protreadmilltestdata.h"
#include "Devices/KingsmithR2Treadmill/kingsmithr2treadmilltestdata.h"
#include "kingsmithr1protreadmill.h"

void KingsmithR1ProTreadmillTestData::configureExclusions() {
    this->exclude(new KingsmithR2TreadmillTestData());
}

KingsmithR1ProTreadmillTestData::KingsmithR1ProTreadmillTestData() : TreadmillTestData("Kingsmith R1 Pro Treadmill") {

    this->addDeviceName("R1 PRO", comparison::StartsWithIgnoreCase);
    this->addDeviceName("RE", comparison::IgnoreCase);
    this->addDeviceName("KINGSMITH", comparison::StartsWithIgnoreCase);
    this->addDeviceName("KS-H", comparison::StartsWithIgnoreCase);
    this->addDeviceName("DYNAMAX", comparison::StartsWithIgnoreCase);
    this->addDeviceName("WALKINGPAD", comparison::StartsWithIgnoreCase);
}

deviceType KingsmithR1ProTreadmillTestData::get_expectedDeviceType() const { return deviceType::KingsmithR1ProTreadmill; }

bool KingsmithR1ProTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<kingsmithr1protreadmill*>(detectedDevice)!=nullptr;
}
