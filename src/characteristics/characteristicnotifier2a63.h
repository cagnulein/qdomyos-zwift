#ifndef CHARACTERISTICNOTIFIER2A63_H
#define CHARACTERISTICNOTIFIER2A63_H

#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2A63 : public CharacteristicNotifier {
    bluetoothdevice *Bike;
    bool garminBluetoothCompatibility;

  public:
    explicit CharacteristicNotifier2A63(bluetoothdevice *Bike, QObject *parent = nullptr,
                                        bool garminBluetoothCompatibility = false);
    int notify(QByteArray &out) override;
};

#endif // CHARACTERISTICNOTIFIER2A63_H
