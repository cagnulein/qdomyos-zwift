#include "ftmsbiketestdata.h"

void FTMSBikeTestData::configureExclusions() {
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new StagesBike1TestData());
    this->exclude(new StagesBike2TestData());
}
