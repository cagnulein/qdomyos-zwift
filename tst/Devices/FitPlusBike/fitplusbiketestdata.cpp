#include "fitplusbiketestdata.h"

#include "Devices/FTMSBike/ftmsbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"


void FitPlusBikeMRKTestData::configureExclusions() {
    this->exclude(new FTMSBike1TestData());
    this->exclude(new FTMSBike2TestData());
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
}
