#ifndef CHARACTERISTICNOTIFIER0002_H
#define CHARACTERISTICNOTIFIER0002_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"

class CharacteristicNotifier0002 : public CharacteristicNotifier {
    Q_OBJECT
    bluetoothdevice* Bike = nullptr;    

public:
    explicit CharacteristicNotifier0002(bluetoothdevice *bike, QObject *parent = nullptr);
    int notify(QByteArray &value) override;
    QByteArray answer;
};

#endif // CHARACTERISTICNOTIFIER0002_H
