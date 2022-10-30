#include "trxappgateusbtreadmilltestdata.h"

#include "Devices/TrxAppGateUSBBike/trxappgateusbbiketestdata.h"


TrxAppGateUSBTreadmillTestData::TrxAppGateUSBTreadmillTestData() {

    this->addDeviceName("TOORX", comparison::StartsWith);
    this->addDeviceName("V-RUN", comparison::StartsWith);

    this->addDeviceName("I-CONSOLE+", comparison::StartsWithIgnoreCase);
    this->addDeviceName("ICONSOLE+", comparison::StartsWithIgnoreCase);
    this->addDeviceName("I-RUNNING", comparison::StartsWithIgnoreCase);
    this->addDeviceName("DKN RUN", comparison::StartsWithIgnoreCase);
    this->addDeviceName("REEBOK", comparison::StartsWithIgnoreCase);


    this->exclude(new TrxAppGateUSBBike1TestData());
    this->exclude(new TrxAppGateUSBBike2TestData());
}
