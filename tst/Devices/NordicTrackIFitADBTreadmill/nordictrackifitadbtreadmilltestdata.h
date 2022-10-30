#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "nordictrackifitadbtreadmill.h"

class NordicTrackIFitADBTreadmillTestData : public BluetoothDeviceTestData {

public:
    NordicTrackIFitADBTreadmillTestData() {
        this->hasSettings = true;

        // Allow any name because it's not a bluetooth device
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    void configureSettings(devicediscoveryinfo& info, bool enable) const override {
        if(enable)
            info.nordictrack_2950_ip = this->get_testIP();
        else
            info.nordictrack_2950_ip = QString();
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NordicTrackIFitADBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nordictrackifitadbtreadmill*>(detectedDevice)!=nullptr;
    }
};

