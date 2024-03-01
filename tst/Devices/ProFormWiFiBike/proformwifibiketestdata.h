#pragma once


#include "Devices/Bike/biketestdata.h"

#include "devices/proformwifibike/proformwifibike.h"


class ProFormWiFiBikeTestData : public BikeTestData {
protected:
    bike* doCreateInstance(const BikeOptions& options) override {
        return new proformwifibike(options.noResistance, options.noHeartService, options.resistanceOffset, options.resistanceGain);
    }

    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.proformtdf4ip = enable ? this->get_testIP():QString();
        return true;
    }
public:
    ProFormWiFiBikeTestData(): BikeTestData("ProForm WiFi Bike") {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::ProformWifiBike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<proformwifibike*>(detectedDevice)!=nullptr;
    }
};

