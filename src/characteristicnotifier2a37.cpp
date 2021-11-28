#include "characteristicnotifier2a37.h"
#include <stdlib.h>

CharacteristicNotifier2A37::CharacteristicNotifier2A37(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(parent), Bike(Bike) {}

int CharacteristicNotifier2A37::notify(QByteArray &valueHR) {
    uint8_t hr = Bike->metrics_override_heartrate();
    valueHR.append(char(0));                       // Flags that specify the format of the value.
    valueHR.append(char(!hr ? rand() % 200 : hr)); // Actual value.
    return CN_OK;
}
