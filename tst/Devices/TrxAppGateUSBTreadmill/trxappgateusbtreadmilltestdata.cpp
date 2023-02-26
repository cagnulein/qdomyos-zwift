#include "trxappgateusbtreadmilltestdata.h"
#include "Devices/TrxAppGateUSBBike/trxappgateusbbiketestdata.h"
#include "trxappgateusbtreadmill.h"

void TrxAppGateUSBTreadmillTestData::configureExclusions() {

    this->exclude(new TrxAppGateUSBBike1TestData());
    this->exclude(new TrxAppGateUSBBike2TestData());
}

void TrxAppGateUSBTreadmillTestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if(enable) {
        config.toorx_bike = false;
        config.toorx_ftms_treadmill = false;
        configurations.push_back(config);
    } else {
        for(int i=1; i<4; i++) {
            config.toorx_bike = i&1;
            config.toorx_ftms_treadmill = i&2;
            configurations.push_back(config);
        }
    }
}

TrxAppGateUSBTreadmillTestData::TrxAppGateUSBTreadmillTestData() : TreadmillTestData("Toorx AppGate USB Treadmill") {
    this->addDeviceName("TOORX", comparison::StartsWith);
    this->addDeviceName("V-RUN", comparison::StartsWith);

    this->addDeviceName("K80_", comparison::StartsWithIgnoreCase);
    this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ICONSOLE+", comparison::StartsWithIgnoreCase);
    this->addDeviceName("I-RUNNING", comparison::StartsWithIgnoreCase);
    this->addDeviceName("DKN RUN", comparison::StartsWithIgnoreCase);
    this->addDeviceName("REEBOK", comparison::StartsWithIgnoreCase);

}

deviceType TrxAppGateUSBTreadmillTestData::get_expectedDeviceType() const { return deviceType::TrxAppGateUSBTreadmill; }

bool TrxAppGateUSBTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<trxappgateusbtreadmill*>(detectedDevice)!=nullptr;
}
