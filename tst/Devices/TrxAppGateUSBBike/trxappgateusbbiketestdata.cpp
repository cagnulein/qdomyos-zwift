#include "trxappgateusbbiketestdata.h" 
#include "Devices/TrxAppGateUSBTreadmill/trxappgateusbtreadmilltestdata.h"
#include "trxappgateusbbike.h"

TrxAppGateUSBBikeTestData::TrxAppGateUSBBikeTestData(std::string testName) : BikeTestData(testName) {

}

void TrxAppGateUSBBikeTestData::configureExclusions() {
    this->exclude(new TrxAppGateUSBTreadmillTestData);
}

deviceType TrxAppGateUSBBikeTestData::get_expectedDeviceType() const { return deviceType::TrxAppGateUSBBike; }

bool TrxAppGateUSBBikeTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<trxappgateusbbike*>(detectedDevice)!=nullptr;
}

void TrxAppGateUSBBike1TestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    // This particular case of TrxAppGateUSBBike is independant of the setting

    DeviceDiscoveryInfo config(info);
    config.toorx_bike = true;
    config.toorx_ftms_treadmill = !enable;
    configurations.push_back(config);

    config.toorx_bike = false;
    config.toorx_ftms_treadmill = !enable;
    configurations.push_back(config);
}

TrxAppGateUSBBike1TestData::TrxAppGateUSBBike1TestData() : TrxAppGateUSBBikeTestData("Toorx AppGate USB Bike")  {
    this->addDeviceName("TUN ", comparison::StartsWithIgnoreCase);
}

void TrxAppGateUSBBike2TestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if(enable) {
        config.toorx_bike = true;
        config.toorx_ftms_treadmill = false;
        configurations.push_back(config);
    } else {
        config.toorx_bike = false;
        config.toorx_ftms_treadmill = true;
        configurations.push_back(config);
        config.toorx_bike = false;
        config.toorx_ftms_treadmill = false;
        configurations.push_back(config);
    }
}

TrxAppGateUSBBike2TestData::TrxAppGateUSBBike2TestData() : TrxAppGateUSBBikeTestData("Toorx AppGate USB Bike (enabled in settings)") {

    this->addDeviceName("TOORX", comparison::StartsWith);
    this->addDeviceName("I-CONSOIE+", comparison::StartsWithIgnoreCase) ;
    this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase) ;
    this->addDeviceName("IBIKING+", comparison::StartsWithIgnoreCase) ;
    this->addDeviceName("ICONSOLE+", comparison::StartsWithIgnoreCase) ;
    this->addDeviceName("VIFHTR2.1", comparison::StartsWithIgnoreCase) ;
    this->addDeviceName("DKN MOTION", comparison::StartsWithIgnoreCase);
    this->addDeviceName("CR011R", comparison::IgnoreCase);
}
