#ifndef CHARACTERISTICNOTIFIER0002_H
#define CHARACTERISTICNOTIFIER0002_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"
#include <QList>

class CharacteristicNotifier0002 : public CharacteristicNotifier {
    Q_OBJECT
    bluetoothdevice* Bike = nullptr;    
    QList<QByteArray> answerList;

public:
    explicit CharacteristicNotifier0002(bluetoothdevice *bike, QObject *parent = nullptr);
    int notify(QByteArray &value) override;
    void addAnswer(const QByteArray &newAnswer);
};

#endif // CHARACTERISTICNOTIFIER0002_H