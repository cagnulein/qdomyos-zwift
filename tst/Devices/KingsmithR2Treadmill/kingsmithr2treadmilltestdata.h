#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "kingsmithr2treadmill.h"

class KingsmithR2TreadmillTestData : public BluetoothDeviceTestData {

public:
    KingsmithR2TreadmillTestData() {
        this->addDeviceName("KS-ST-K12PRO", true, true);

        // KingSmith Walking Pad R2
        this->addDeviceName("KS-R1AC", true, true);
        this->addDeviceName("KS-HC-R1AA", true, true);
        this->addDeviceName("KS-HC-R1AC", true, true);

        // KingSmith Walking Pad X21
        this->addDeviceName("KS-X21", true, true);
        this->addDeviceName("KS-HDSC-X21C", true, true);
        this->addDeviceName("KS-HDSY-X21C", true, true);
        this->addDeviceName("KS-NGCH-X21C", true, true);

    }


    deviceType get_expectedDeviceType() const override { return deviceType::KingsmithR2Treadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<kingsmithr2treadmill*>(detectedDevice)!=nullptr;
    }
};

