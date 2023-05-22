#pragma once

#include "Devices/Treadmill/treadmilltestdata.h"
#include "kingsmithr2treadmill.h"

class KingsmithR2TreadmillTestData : public TreadmillTestData {

public:
    KingsmithR2TreadmillTestData() : TreadmillTestData("Kingsmith R2 Treadmill") {
        this->addDeviceName("KS-ST-K12PRO", comparison::StartsWithIgnoreCase);

        // KingSmith Walking Pad R2
        this->addDeviceName("KS-R1AC", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-HC-R1AA", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-HC-R1AC", comparison::StartsWithIgnoreCase);

        // KingSmith Walking Pad X21
        this->addDeviceName("KS-X21", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-HDSC-X21C", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-HDSY-X21C", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-NGCH-X21C", comparison::StartsWithIgnoreCase);
        this->addDeviceName("KS-NACH-X21C", comparison::StartsWithIgnoreCase);

    }


    deviceType get_expectedDeviceType() const override { return deviceType::KingsmithR2Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<kingsmithr2treadmill*>(detectedDevice)!=nullptr;
    }
};

