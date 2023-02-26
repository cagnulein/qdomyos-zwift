#include "paferstreadmilltestdata.h" 
#include "paferstreadmill.h"

void PafersTreadmillTestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if (enable) {
        for(int x = 1; x<=3; x++) {
            config.pafers_treadmill = x & 1;
            config.pafers_treadmill_bh_iboxster_plus = x & 2;
            configurations.push_back(config);
        }
    } else {
        config.pafers_treadmill = false;
        config.pafers_treadmill_bh_iboxster_plus = false;
        configurations.push_back(config);
    }

}

PafersTreadmillTestData::PafersTreadmillTestData() : TreadmillTestData("Pafers Treadmill") {
    this->addDeviceName("PAFERS_", comparison::StartsWithIgnoreCase);
}

deviceType PafersTreadmillTestData::get_expectedDeviceType() const { return deviceType::PafersTreadmill; }

bool PafersTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<paferstreadmill*>(detectedDevice)!=nullptr;
}
