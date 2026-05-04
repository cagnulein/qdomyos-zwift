#ifndef CHARACTERISTICNOTIFIER0004_H
#define CHARACTERISTICNOTIFIER0004_H

#include "bluetoothdevice.h"
#include "characteristicnotifier.h"
#include <QList>

class CharacteristicNotifier0004 : public CharacteristicNotifier {
    Q_OBJECT
    bluetoothdevice* Bike = nullptr;    
    QList<QByteArray> answerList;

public:
    explicit CharacteristicNotifier0004(bluetoothdevice *bike, QObject *parent = nullptr);
    int notify(QByteArray &value) override;
    void addAnswer(const QByteArray &newAnswer);
};

#endif // CHARACTERISTICNOTIFIER0004_H