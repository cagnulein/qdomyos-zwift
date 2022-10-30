#include "fitplusbiketestdata.h"

#include "Devices/FTMSBike/ftmsbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"

FitPlusBikeMRKTestData::FitPlusBikeMRKTestData() {

    this->addDeviceName("MRK-", comparison::StartsWith);

    this->exclude(new FTMSBikeTestData());
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
}
