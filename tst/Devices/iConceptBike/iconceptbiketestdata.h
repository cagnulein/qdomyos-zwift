#pragma once

#include "Devices/Bike/biketestdata.h"

class iConceptBikeTestData : public BikeTestData {
protected:
void configureSettings(const DeviceDiscoveryInfo &info, bool enable,
                           std::vector<DeviceDiscoveryInfo> &configurations) const override;

  public:
    iConceptBikeTestData();

    deviceType get_expectedDeviceType() const override;

    bool get_isExpectedDevice(bluetoothdevice * detectedDevice) const override;
};

