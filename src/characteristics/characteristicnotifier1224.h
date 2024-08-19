#ifndef CHARACTERISTICNOTIFIER1224_H
#define CHARACTERISTICNOTIFIER1224_H

#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier1224 : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:
    explicit CharacteristicNotifier1224(bluetoothdevice *Bike, QObject *parent = nullptr);
    int notify(QByteArray &out) override;
};

#endif // CHARACTERISTICNOTIFIER1224_H
