#include "characteristicnotifier0002.h"
#include "devices/elliptical.h"
#include "devices/rower.h"
#include "devices/treadmill.h"
#include <QSettings>

static uint8_t sequenceChar1224 = 0;

CharacteristicNotifier0002::CharacteristicNotifier0002(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2ad2, parent), Bike(Bike) {}

int CharacteristicNotifier0002::notify(QByteArray &value) {
    bluetoothdevice::BLUETOOTH_TYPE dt = Bike->deviceType();

    return CN_OK;
}
