#ifndef CHARACTERISTICNOTIFIES2ACC_H
#define CHARACTERISTICNOTIFIES2ACC_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2ACC : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:
    explicit CharacteristicNotifier2ACC(bluetoothdevice *Bike, QObject *parent = nullptr);
    virtual int notify(QByteArray &out);
};

#endif // CHARACTERISTICNOTIFIES2ACC_H
