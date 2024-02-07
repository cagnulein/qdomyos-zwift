#pragma once

#include "Devices/bluetoothdevicetestdata.h"
#include "trixterxdreamv1bike.h"
#include "testserialdatasource.h"

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

        // use the test serial data source because the bike won't be there usually, during test runs.
        trixterxdreamv1serial::serialDataSourceFactory = TestSerialDatasource::create;
    }

    deviceType get_expectedDeviceType() const override { return deviceType::TrixterXDreamV1Bike; }

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override {
        return dynamic_cast<trixterxdreamv1bike*>(detectedDevice)!=nullptr;
    }
};

