#include "fitshowtreadmilltestdata.h" 
#include "fitshowtreadmill.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

FitshowTreadmillTestData::FitshowTreadmillTestData(std::string testName) : TreadmillTestData(testName) {

}

deviceType FitshowTreadmillTestData::get_expectedDeviceType() const { return deviceType::FitshowTreadmill; }

bool FitshowTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<fitshowtreadmill*>(detectedDevice)!=nullptr;
}

void FitshowTreadmillFSTestData::configureSettings(const DeviceDiscoveryInfo &info, bool enable, std::vector<DeviceDiscoveryInfo> &configurations) const {
    DeviceDiscoveryInfo config(info);

    if(enable){
        config.snode_bike = false;
        config.fitplus_bike = false;
        configurations.push_back(config);
    } else {
        for(int i=1; i<4; i++) {
            config.snode_bike = i&1;
            config.fitplus_bike = i&2;
            configurations.push_back(config);
        }
    }
}

FitshowTreadmillFSTestData::FitshowTreadmillFSTestData() : FitshowTreadmillTestData("FitShow FS") {
    this->addDeviceName("FS-", comparison::StartsWith);
}

void FitshowTreadmillFSTestData::configureExclusions() {
    this->exclude(new FTMSBike1TestData());
    this->exclude(new FTMSBike2TestData());
}

FitshowTreadmillSWTestData::FitshowTreadmillSWTestData(): FitshowTreadmillTestData("FitShow SW") {
    // SW, 14 characters total
    this->addDeviceName("SW345678901234", comparison::Exact);
    this->addDeviceName("SWFOURTEENCHAR", comparison::Exact);
    this->addDeviceName("WINFITA", comparison::StartsWithIgnoreCase);
    this->addDeviceName("NOBLEPRO CONNECT", comparison::StartsWithIgnoreCase);

    // too long and too short
    this->addInvalidDeviceName("SW3456789012345", comparison::Exact);
    this->addInvalidDeviceName("SW34567890123", comparison::Exact);

}
