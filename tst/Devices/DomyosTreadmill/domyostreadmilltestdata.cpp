#include "domyostreadmilltestdata.h"

#include "devices/domyostreadmill/domyostreadmill.h"

#include "Devices/DomyosElliptical/domyosellipticaltestdata.h"
#include "Devices/DomyosRower/domyosrowertestdata.h"
#include "Devices/DomyosBike/domyosbiketestdata.h"
#include "Devices/HorizonTreadmill/horizontreadmilltestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

DomyosTreadmillTestData::DomyosTreadmillTestData() : TreadmillTestData("Domyos Treadmill") {
    this->addDeviceName("Domyos", comparison::StartsWith);

    this->addInvalidDeviceName("DomyosBr", comparison::StartsWith);
}

void DomyosTreadmillTestData::configureExclusions() {
    this->exclude(new DomyosEllipticalTestData());
    this->exclude(new DomyosBikeTestData());
    this->exclude(new DomyosRowerTestData());
    this->exclude(new HorizonTreadmillTestData());
    this->exclude(new HorizonTreadmillToorxTestData());
    this->exclude(new HorizonTreadmillBodyToneTestData());
    this->exclude(new HorizonTreadmillDomyosTCTestData());
    this->exclude(new FTMSBike1TestData());
    this->exclude(new FTMSBike2TestData());
    this->exclude(new FTMSBike3TestData());
    this->exclude(new FTMSBike4TestData());
}

deviceType DomyosTreadmillTestData::get_expectedDeviceType() const { return deviceType::DomyosTreadmill; }

bool DomyosTreadmillTestData::get_isExpectedDevice(bluetoothdevice *detectedDevice) const {
    return dynamic_cast<domyostreadmill*>(detectedDevice)!=nullptr;
}


void DomyosTreadmillTestData::configureBluetoothDeviceInfos(const QBluetoothDeviceInfo &info, bool enable, std::vector<QBluetoothDeviceInfo> &bluetoothDeviceInfos) const {

    if(enable) return;

    // Should not be identified if it has 0x1826
    auto result = info;
    result.setServiceUuids(QVector<QBluetoothUuid>({QBluetoothUuid((quint16)0x1826)}));
    bluetoothDeviceInfos.push_back(result);


}

