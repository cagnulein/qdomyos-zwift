#include "renphobiketestdata.h"
#include "Devices/FitPlusBike/fitplusbiketestdata.h"
#include "Devices/SnodeBike/snodebiketestdata.h"

void RenphoBikeTestData::configureExclusions() {
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
}
