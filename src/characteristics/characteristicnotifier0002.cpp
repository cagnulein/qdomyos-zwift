#include "characteristicnotifier0002.h"
#include "bike.h"
#include <QDebug>
#include <QList>

CharacteristicNotifier0002::CharacteristicNotifier0002(bluetoothdevice *bike, QObject *parent)
    : CharacteristicNotifier(0x0002, parent) {
    Bike = bike;
    answerList = QList<QByteArray>();  // Initialize empty list
}

void CharacteristicNotifier0002::addAnswer(const QByteArray &newAnswer) {
    answerList.append(newAnswer);
}

int CharacteristicNotifier0002::notify(QByteArray &value) {
    if(!answerList.isEmpty()) {
        value.append(answerList.first());  // Get first item
        answerList.removeFirst();          // Remove it from list
        return CN_OK;
    }
    return CN_INVALID;
}