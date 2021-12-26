#include "characteristicnotifier2a53.h"
#include "treadmill.h"

CharacteristicNotifier2A53::CharacteristicNotifier2A53(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2a53, parent), Bike(Bike) {}

int CharacteristicNotifier2A53::notify(QByteArray &value) {
    bluetoothdevice::BLUETOOTH_TYPE dt = Bike->deviceType();
    if (dt == bluetoothdevice::TREADMILL || dt == bluetoothdevice::ELLIPTICAL) {
        value.append(0x02); // total distance
        uint16_t speed = Bike->currentSpeed().value() / 3.6;
        uint32_t distance = Bike->odometer() * 1000.0;
        value.append((char)((speed & 0xFF)));
        value.append((char)((speed >> 8) & 0xFF));
        value.append((char)(90)); // cadence, i don't have any sensor to measure it
        value.append((char)((distance & 0xFF)));
        value.append((char)((distance >> 8) & 0xFF));
        value.append((char)((distance >> 16) & 0xFF));
        value.append((char)((distance >> 24) & 0xFF));
        return CN_OK;
    } else
        return CN_INVALID;
}
