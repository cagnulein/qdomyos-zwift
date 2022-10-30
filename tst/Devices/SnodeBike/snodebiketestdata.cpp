#include "snodebiketestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

void SnodeBikeTestData::configureExclusions() {
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new FTMSBike1TestData());
    this->exclude(new FTMSBike2TestData());
}
