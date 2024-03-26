#include "bikeergfunctions.h"

void bikeergfunctions::setCadence(int32_t cadence) { this->device->cadenceSensor(cadence); }

bikeergfunctions::bikeergfunctions(bike *device) : device(device) {}

std::optional<int32_t> bikeergfunctions::getMaxCadence() const { return this->device->cadenceLimits().max();}

std::optional<int32_t> bikeergfunctions::getMinCadence() const { return this->device->cadenceLimits().min(); }

std::optional<resistance_t> bikeergfunctions::getMaxResistance() const { return this->device->resistanceLimits().max(); }

std::optional<resistance_t> bikeergfunctions::getMinResistance() const { return this->device->resistanceLimits().min(); }

double bikeergfunctions::getPower(const int32_t cadence, resistance_t resistance) {
    auto originalCadence = this->device->currentCadence().value();

    this->setCadence(cadence);
    auto result = this->device->powerFromResistanceRequest(resistance);
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
