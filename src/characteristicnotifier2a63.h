#ifndef CHARACTERISTICNOTIFIER2A63_H
#define CHARACTERISTICNOTIFIER2A63_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2A63 : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:
    explicit CharacteristicNotifier2A63(bluetoothdevice *Bike, QObject *parent = nullptr);
    virtual int notify(QByteArray &out);
};

#endif // CHARACTERISTICNOTIFIER2A63_H
