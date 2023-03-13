#include "testlockscreen.h"

void TestLockscreen::reset() {
    this->configType = QZLockscreenFunctions::configurationType::NONE;
    this->zwiftMode = false;
}

QZLockscreenFunctions::configurationType TestLockscreen::get_virtualDeviceType() const { return this->configType; }

bool TestLockscreen::get_zwiftMode() { return this->zwiftMode; }

TestLockscreen::TestLockscreen() {
    this->reset();
}

TestLockscreen::~TestLockscreen() {}

void TestLockscreen::setTimerDisabled() {  }

void TestLockscreen::request() {}

long TestLockscreen::heartRate() { return 0; }

long TestLockscreen::stepCadence() { return 0; }

void TestLockscreen::setKcal(double kcal) {}

void TestLockscreen::setDistance(double distance) {}

void TestLockscreen::virtualbike_ios() { this->configType = QZLockscreenFunctions::BIKE; this->zwiftMode = false;}

void TestLockscreen::virtualbike_setHeartRate(unsigned char heartRate) {}

void TestLockscreen::virtualbike_setCadence(unsigned short crankRevolutions, unsigned short lastCrankEventTime) {}

void TestLockscreen::virtualbike_zwift_ios(bool disableHeartRate) {this->configType = QZLockscreenFunctions::BIKE; this->zwiftMode = true;}

double TestLockscreen::virtualbike_getCurrentSlope() { return 0.0;}

double TestLockscreen::virtualbike_getCurrentCRR() {return 0.0;}

double TestLockscreen::virtualbike_getCurrentCW() {return 0.0;}

double TestLockscreen::virtualbike_getPowerRequested() {return 0.0;}

bool TestLockscreen::virtualbike_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt, unsigned short CrankRevolutions, unsigned short LastCrankEventTime) { return false;}

int TestLockscreen::virtualbike_getLastFTMSMessage(unsigned char *message) { return 0;}

void TestLockscreen::virtualrower_ios() {this->configType = QZLockscreenFunctions::ROWER; this->zwiftMode = false;}

void TestLockscreen::virtualrower_setHeartRate(unsigned char heartRate) {}

bool TestLockscreen::virtualrower_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt, unsigned short CrankRevolutions, unsigned short LastCrankEventTime, unsigned short StrokesCount, unsigned int Distance, unsigned short KCal, unsigned short Pace) { return false; }

int TestLockscreen::virtualrower_getLastFTMSMessage(unsigned char *message) { return 0; }

void TestLockscreen::virtualtreadmill_zwift_ios() { this->configType = QZLockscreenFunctions::TREADMILL; this->zwiftMode = true;}

void TestLockscreen::virtualtreadmill_setHeartRate(unsigned char heartRate) {}

double TestLockscreen::virtualtreadmill_getCurrentSlope() { return 0.0;}

uint64_t TestLockscreen::virtualtreadmill_lastChangeCurrentSlope() { return 0;}

double TestLockscreen::virtualtreadmill_getPowerRequested() { return 0.0;}

bool TestLockscreen::virtualtreadmill_updateFTMS(unsigned short normalizeSpeed, unsigned char currentResistance, unsigned short currentCadence, unsigned short currentWatt, unsigned short currentInclination) { return false;}

double TestLockscreen::getVolume() { return 0.0;}


