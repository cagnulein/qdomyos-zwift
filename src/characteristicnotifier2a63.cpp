#include "characteristicnotifier2a63.h"

CharacteristicNotifier2A63::CharacteristicNotifier2A63(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(parent), Bike(Bike) {}

int CharacteristicNotifier2A63::notify(QByteArray &value) {
    value.append((char)0x20); // crank data present
    value.append((char)0x00);
    value.append((char)(((uint16_t)Bike->wattsMetric().value()) & 0xFF));          // watt
    value.append((char)(((uint16_t)Bike->wattsMetric().value()) >> 8) & 0xFF);     // watt
    value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) & 0xFF));      // revs count
    value.append((char)(((uint16_t)Bike->currentCrankRevolutions()) >> 8) & 0xFF); // revs count
    value.append((char)(Bike->lastCrankEventTime() & 0xff));                       // eventtime
    value.append((char)(Bike->lastCrankEventTime() >> 8) & 0xFF);                  // eventtime
    return CN_OK;
}
