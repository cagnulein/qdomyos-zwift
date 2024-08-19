#include "characteristicnotifier1224.h"
#include "devices/elliptical.h"
#include "devices/rower.h"
#include "devices/treadmill.h"
#include <QSettings>

static uint8_t sequenceChar1224 = 0;

CharacteristicNotifier1224::CharacteristicNotifier1224(bluetoothdevice *Bike, QObject *parent)
    : CharacteristicNotifier(0x2ad2, parent), Bike(Bike) {}

int CharacteristicNotifier1224::notify(QByteArray &value) {
    bluetoothdevice::BLUETOOTH_TYPE dt = Bike->deviceType();

    ++sequenceChar1224;

    value.append((char)sequenceChar1224);
    value.append((char)0x03);

    value.append((char)(0xb6));
    value.append((char)(0x01));
    return CN_OK;
}
