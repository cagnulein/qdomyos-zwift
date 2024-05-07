#pragma once
#include "Devices/Elliptical/ellipticaltestdata.h"
#include "devices/iconceptelliptical/iconceptelliptical.h"

class iConceptEllipticalTestData : public EllipticalTestData {
    void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override {
        DeviceDiscoveryInfo config(info);

        if (enable) {
            config.iconcept_elliptical = true;
            configurations.push_back(config);
        } else {
            config.iconcept_elliptical = false;
            configurations.push_back(config);
        }
    }

  public:
    iConceptEllipticalTestData() : EllipticalTestData("iConcept Elliptical") {
        this->addDeviceName("BH DUALKIT", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::IConceptElliptical; }

    bool get_isExpectedDevice(bluetoothdevice *detectedDevice) const override {
        return dynamic_cast<iconceptelliptical *>(detectedDevice) != nullptr;
    }
};

