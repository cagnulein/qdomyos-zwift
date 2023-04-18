#pragma once
#include "Devices/Elliptical/ellipticaltestdata.h"
#include "Devices/bluetoothdevicetestdata.h"
#include "fakeelliptical.h"

class FakeEllipticalTestData : public EllipticalTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.fakedevice_elliptical = enable;
        return true;
    }

    void configureLockscreenSettings(const DeviceDiscoveryInfo &info, std::vector<LockscreenFunctionsTestData> &configurations) const override {
        DeviceDiscoveryInfo config(info);
        auto virtualDevice = QZLockscreenFunctions::configurationType::BIKE;

        for(int i=0; i<8; i++) {
            config.ios_peloton_workaround = i&1;
            config.bike_cadence_sensor = i&2;
            config.virtual_device_rower = i&4;
            bool pelotonEnabled = config.ios_peloton_workaround && config.bike_cadence_sensor; // virtual_device_rower not consdered by this rower
            configurations.push_back(LockscreenFunctionsTestData(virtualDevice, pelotonEnabled, false, config));
        }
    }
public:
    FakeEllipticalTestData() : EllipticalTestData("Fake Elliptical") {
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::FakeElliptical; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<fakeelliptical*>(detectedDevice)!=nullptr;
    }
};

