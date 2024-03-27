#include "ftmsbiketestdata.h"

#include "Devices/SnodeBike/snodebiketestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/StagesBike/stagesbiketestdata.h"

void FTMSBikeTestData::configureExclusions() {
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new StagesBike1TestData());
    this->exclude(new StagesBike2TestData());
}
