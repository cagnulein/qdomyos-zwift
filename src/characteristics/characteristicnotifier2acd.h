#ifndef CHARACTERISTICNOTIFIER2ACD_H
#define CHARACTERISTICNOTIFIER2ACD_H
#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2ACD : public CharacteristicNotifier {
    bluetoothdevice *Bike;
    const bool includeDistance;

  public:
    explicit CharacteristicNotifier2ACD(bluetoothdevice *Bike, QObject *parent = nullptr, bool includeDistance = true);
    int notify(QByteArray &out) override;
};

#endif // CHARACTERISTICNOTIFIER2ACD_H
