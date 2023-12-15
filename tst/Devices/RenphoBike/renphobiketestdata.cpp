#include "renphobiketestdata.h"

void RenphoBikeTestData::configureExclusions() {
    this->exclude(new FitPlusBikeFSTestData());
    this->exclude(new FitPlusBikeMRKTestData());
    this->exclude(new SnodeBike1TestData());
    this->exclude(new SnodeBike2TestData());
}
