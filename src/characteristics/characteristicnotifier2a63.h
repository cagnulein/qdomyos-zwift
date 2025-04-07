#ifndef CHARACTERISTICNOTIFIER2A63_H
#define CHARACTERISTICNOTIFIER2A63_H

#include "devices/bluetoothdevice.h"
#include "characteristicnotifier.h"
#include <QSettings>

class CharacteristicNotifier2A63 : public CharacteristicNotifier {
    bluetoothdevice *Bike;
    uint16_t revolutions = 0;
    uint16_t timestamp = 0;
    uint64_t lastRevolutionTimestamp = 0;

  public:
    explicit CharacteristicNotifier2A63(bluetoothdevice *Bike, QObject *parent = nullptr);
    int notify(QByteArray &out) override;
};

#endif // CHARACTERISTICNOTIFIER2A63_H
