#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "trixterxdreamv1bike.h"

class TrixterXDreamV1BikeTestData : public BluetoothDeviceTestData {
protected:
    bool configureSettings(DeviceDiscoveryInfo& info, bool enable) const override {
        info.trixter_xdream_v1_bike = enable;
        return true;
    }
public:
    TrixterXDreamV1BikeTestData(): BluetoothDeviceTestData("Trixter X-Dream V1 Bike") {
        // any name
        this->addDeviceName("", comparison::StartsWithIgnoreCase);
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrixterXDreamV1Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trixterxdreamv1bike*>(detectedDevice)!=nullptr;
    }
};

