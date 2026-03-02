#include "characteristicnotifier2acc.h"
#include "devices/treadmill.h"
#include <qmath.h>

CharacteristicNotifier2ACC::CharacteristicNotifier2ACC(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2ACC, parent), Bike(Bike) {}

int CharacteristicNotifier2ACC::notify(QByteArray &value) {
    value.append((char)0x83); // average speed, cadence and resistance level supported
    value.append((char)0x14); // heart rate and elapsed time
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x0F); // resistance, power, speed and inclination target supported
    value.append((char)0xE0); // indoor simulation, wheel and spin down supported
    value.append((char)0x00);
    value.append((char)0x00);
    return CN_OK;
}
