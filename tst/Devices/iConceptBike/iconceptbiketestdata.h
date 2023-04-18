#pragma once

#include "Devices/Bike/biketestdata.h"
#include "iconceptbike.h"

class iConceptBikeTestData : public BikeTestData {

void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            config.iconcept_elliptical = false;
            configurations.push_back(config);
        } else {
            config.iconcept_elliptical = true;
            configurations.push_back(config);
        }
    }

  public:
    iConceptBikeTestData() : BikeTestData("iConcept Bike") {
        this->addDeviceName("BH DUALKIT", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::IConceptBike; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<iconceptbike *>(detectedDevice) != nullptr;
    }
};

