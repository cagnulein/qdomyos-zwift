#include "characteristicnotifier2ad2.h"

CharacteristicNotifier2AD2::CharacteristicNotifier2AD2(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(parent), Bike(Bike) {}

int CharacteristicNotifier2AD2::notify(QByteArray &value) {
    uint16_t normalizeSpeed = (uint16_t)qRound(Bike->currentSpeed().value() * 100);
    value.append((char)0x64); // speed, inst. cadence, resistance lvl, instant power
    value.append((char)0x02); // heart rate

    value.append((char)(normalizeSpeed & 0xFF));      // speed
    value.append((char)(normalizeSpeed >> 8) & 0xFF); // speed

    value.append((char)((uint16_t)(Bike->currentCadence().value() * 2) & 0xFF));        // cadence
    value.append((char)(((uint16_t)(Bike->currentCadence().value() * 2) >> 8) & 0xFF)); // cadence

    value.append((char)Bike->currentResistance().value()); // resistance
    value.append((char)(0));                               // resistance

    value.append((char)(((uint16_t)Bike->wattsMetric().value()) & 0xFF));      // watts
    value.append((char)(((uint16_t)Bike->wattsMetric().value()) >> 8) & 0xFF); // watts

    value.append(char(Bike->currentHeart().value())); // Actual value.
    value.append((char)0);
    return CN_OK;
}
