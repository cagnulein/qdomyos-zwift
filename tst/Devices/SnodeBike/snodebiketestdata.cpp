#include "snodebiketestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/FTMSBike/ftmsbiketestdata.h"

SnodeBikeTestData::SnodeBikeTestData() {
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new FTMSBikeTestData());
}
