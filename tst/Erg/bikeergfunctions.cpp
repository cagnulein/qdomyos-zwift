#include "bikeergfunctions.h"

void bikeergfunctions::setCadence(int32_t cadence) { this->device->cadenceSensor(cadence); }

bikeergfunctions::bikeergfunctions(bike *device) : device(device) {}

double bikeergfunctions::getPower(const int32_t cadence, resistance_t resistance) {
    auto originalCadence = this->device->currentCadence().value();

    this->setCadence(cadence);
    auto result = this->device->wattsFromResistance(resistance);
    this->setCadence(originalCadence);
    return result;
}

int32_t bikeergfunctions::getResistance(const int32_t cadence, const double power) {
    auto originalCadence = this->device->currentCadence().value();
    this->setCadence(cadence);
    auto result = this->device->resistanceFromPowerRequest(power);
    this->setCadence(originalCadence);
    return result;
}

double bikeergfunctions::toPeloton(const resistance_t resistance) { return device->bikeResistanceToPeloton(resistance); }

resistance_t bikeergfunctions::fromPeloton(const int pelotonResistance) { return device->pelotonToBikeResistance(pelotonResistance); }


