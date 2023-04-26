#ifndef CHARACTERISTICNOTIFIER2A5B_H
#define CHARACTERISTICNOTIFIER2A5B_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier2A5B : public CharacteristicNotifier {
    bluetoothdevice *Bike;
    uint16_t lastWheelTime = 0;
    uint32_t wheelRevs = 0;
    bool bike_wheel_revs;

  public:
    explicit CharacteristicNotifier2A5B(bluetoothdevice *Bike, QObject *parent = nullptr);
    virtual int notify(QByteArray &out);
};

#endif // CHARACTERISTICNOTIFIER2A5B_H
