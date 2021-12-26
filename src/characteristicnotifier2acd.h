#ifndef CHARACTERISTICNOTIFIER2ACD_H
#define CHARACTERISTICNOTIFIER2ACD_H
#include "bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2ACD : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:
    explicit CharacteristicNotifier2ACD(bluetoothdevice *Bike, QObject *parent = nullptr);
    virtual int notify(QByteArray &out);
};

#endif // CHARACTERISTICNOTIFIER2ACD_H
