#ifndef CHARACTERISTICNOTIFIER2AD9_H
#define CHARACTERISTICNOTIFIER2AD9_H

#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2AD9 : public CharacteristicNotifier {
    bluetoothdevice *Bike;

  public:    
    explicit CharacteristicNotifier2AD9(bluetoothdevice *Bike, QObject *parent = nullptr);
    int notify(QByteArray &out) override;
    QByteArray answer;
};

#endif // CHARACTERISTICNOTIFIER2A53_H
