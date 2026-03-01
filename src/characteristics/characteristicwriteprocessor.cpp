#include "characteristicwriteprocessor.h"

CharacteristicWriteProcessor::CharacteristicWriteProcessor(double bikeResistanceGain, int8_t bikeResistanceOffset,
                                                           bluetoothdevice *bike, QObject *parent)
    : QObject(parent), bikeResistanceOffset(bikeResistanceOffset), bikeResistanceGain(bikeResistanceGain), Bike(bike) {}

void CharacteristicWriteProcessor::changePower(uint16_t power) { Bike->changePower(power); }

void CharacteristicWriteProcessor::changeSlope(int16_t iresistance, uint8_t crr, uint8_t cw) {
    double grade, percentage;
    Bike->processSlopeChange(iresistance, crr, cw, grade, percentage, bikeResistanceGain, bikeResistanceOffset);
    emit changeInclination(grade, percentage);
    emit slopeChanged();
}
