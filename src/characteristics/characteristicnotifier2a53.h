#ifndef CHARACTERISTICNOTIFIER2A53_H
#define CHARACTERISTICNOTIFIER2A53_H

#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2A53 : public CharacteristicNotifier {
    bluetoothdevice *Bike;
    bool garminFootPod;

  public:
    // garminFootPod: cadence in cycles per minute with a speed-based estimate, and the walking/running flag.
    // Keep it last: DirconManager builds every notifier with (Bike, this) through a macro.
    explicit CharacteristicNotifier2A53(bluetoothdevice *Bike, QObject *parent = nullptr, bool garminFootPod = false);
    int notify(QByteArray &out) override;
};

#endif // CHARACTERISTICNOTIFIER2A53_H
