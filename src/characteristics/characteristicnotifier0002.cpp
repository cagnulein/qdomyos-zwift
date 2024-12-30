#include "characteristicnotifier0002.h"
#include "bike.h"
#include <QDebug>

CharacteristicNotifier0002::CharacteristicNotifier0002(bluetoothdevice *bike, QObject *parent)
    : CharacteristicNotifier(0x0002, parent) {
    Bike = bike;
}

int CharacteristicNotifier0002::notify(QByteArray &value) {
    if(answer.length()) {
        value.append(answer);
        return CN_OK;
    }
    return CN_INVALID;
}
