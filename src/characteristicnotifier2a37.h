#ifndef CHARACTERISTICNOTIFIER2A37_H
#define CHARACTERISTICNOTIFIER2A37_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2A37 : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:
    explicit CharacteristicNotifier2A37(bluetoothdevice *Bike, QObject *parent = nullptr);
    virtual int notify(QByteArray &out);
};

#endif // CHARACTERISTICNOTIFIER2A37_H
