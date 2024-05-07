#pragma once


#include "Devices/Treadmill/treadmilltestdata.h"

#include "devices/nordictrackifitadbtreadmill/nordictrackifitadbtreadmill.h"


class NordicTrackIFitADBTreadmillTestData : public TreadmillTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        if(enable)
            info.nordictrack_2950_ip = this->get_testIP();
        else
            info.nordictrack_2950_ip = QString();

        return true;
    }
public:
    NordicTrackIFitADBTreadmillTestData() : TreadmillTestData("Nordictrack IFit ADB Treadmill") {
        // Allow any name because it's not a bluetooth device
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::NordicTrackIFitADBTreadmill; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<nordictrackifitadbtreadmill*>(detectedDevice)!=nullptr;
    }
};

