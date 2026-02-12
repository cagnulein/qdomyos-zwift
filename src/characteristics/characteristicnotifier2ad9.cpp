#include "characteristicnotifier2ad9.h"
#include "devices/ftmsbike/ftmsbike.h"

CharacteristicNotifier2AD9::CharacteristicNotifier2AD9(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2ad9, parent), Bike(Bike) {}

int CharacteristicNotifier2AD9::notify(QByteArray &value) {
    if(answer.length()) {
        value.append(answer);
        return CN_OK;
    }
    return CN_INVALID;
}
