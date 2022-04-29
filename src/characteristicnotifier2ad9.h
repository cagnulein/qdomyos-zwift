#ifndef CHARACTERISTICNOTIFIER2AD9_H
#define CHARACTERISTICNOTIFIER2AD9_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2AD9 : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:    
    explicit CharacteristicNotifier2AD9(bluetoothdevice *Bike, QObject *parent = nullptr);
    virtual int notify(QByteArray &out);
    QByteArray answer;
};

#endif // CHARACTERISTICNOTIFIER2A53_H
