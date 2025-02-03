#ifndef CHARACTERISTICNOTIFIER2A53_H
#define CHARACTERISTICNOTIFIER2A53_H

#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2A53 : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:
    explicit CharacteristicNotifier2A53(bluetoothdevice *Bike, QObject *parent = nullptr);
    int notify(QByteArray &out) override;
};

#endif // CHARACTERISTICNOTIFIER2A53_H
