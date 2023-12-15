#include "trxappgateusbtreadmilltestdata.h"
#include "Devices/TrxAppGateUSBBike/trxappgateusbbiketestdata.h"

void TrxAppGateUSBTreadmillTestData::configureExclusions() {

    this->exclude(new TrxAppGateUSBBike1TestData());
    this->exclude(new TrxAppGateUSBBike2TestData());
}

TrxAppGateUSBTreadmillTestData::TrxAppGateUSBTreadmillTestData() : BluetoothDeviceTestData("Toorx AppGate USB Treadmill") {
    this->addDeviceName("TOORX", comparison::StartsWith);
    this->addDeviceName("V-RUN", comparison::StartsWith);

    this->addDeviceName("K80_", comparison::StartsWithIgnoreCase);
    this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ICONSOLE+", comparison::StartsWithIgnoreCase);
    this->addDeviceName("I-RUNNING", comparison::StartsWithIgnoreCase);
    this->addDeviceName("DKN RUN", comparison::StartsWithIgnoreCase);
    this->addDeviceName("REEBOK", comparison::StartsWithIgnoreCase);

}
