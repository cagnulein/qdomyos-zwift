#include "characteristicnotifier2a37.h"

CharacteristicNotifier2A37::CharacteristicNotifier2A37(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2a37, parent), Bike(Bike) {}

int CharacteristicNotifier2A37::notify(QByteArray &valueHR) {
    valueHR.append(char(0));                                  // Flags that specify the format of the value.
    valueHR.append(char(Bike->metrics_override_heartrate())); // Actual value.
    return CN_OK;
}
