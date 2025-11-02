#ifndef CHARACTERISTICNOTIFIER2A63_H
#define CHARACTERISTICNOTIFIER2A63_H

#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2A63 : public CharacteristicNotifier {
    bluetoothdevice *Bike;

    // Garmin compatibility state
    uint16_t garmin_revolutions = 0;
    uint16_t garmin_timestamp = 0;
    qint64 garmin_lastRevolution = 0;

  public:
    explicit CharacteristicNotifier2A63(bluetoothdevice *Bike, QObject *parent = nullptr);
    int notify(QByteArray &out) override;
};

#endif // CHARACTERISTICNOTIFIER2A63_H
