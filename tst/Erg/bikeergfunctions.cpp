#include "bikeergfunctions.h"

void bikeergfunctions::setCadence(int32_t cadence) { this->device->cadenceSensor(cadence); }

bikeergfunctions::bikeergfunctions(bike *device) : device(device) {}

std::optional<int32_t> bikeergfunctions::getMaxCadence() const { return std::optional<int32_t>{};}

std::optional<int32_t> bikeergfunctions::getMinCadence() const { return 0; }

std::optional<resistance_t> bikeergfunctions::getMaxResistance() const { return this->device->maxResistance(); }

std::optional<resistance_t> bikeergfunctions::getMinResistance() const { return 0; }

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
