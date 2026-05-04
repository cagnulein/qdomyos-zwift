#include "characteristicnotifier0004.h"
#include "bike.h"
#include <QDebug>
#include <QList>

CharacteristicNotifier0004::CharacteristicNotifier0004(bluetoothdevice *bike, QObject *parent)
    : CharacteristicNotifier(0x0004, parent) {
    Bike = bike;
    answerList = QList<QByteArray>();
}

void CharacteristicNotifier0004::addAnswer(const QByteArray &newAnswer) {
    answerList.append(newAnswer);
}

int CharacteristicNotifier0004::notify(QByteArray &value) {
    if(!answerList.isEmpty()) {
        value.append(answerList.first());
        answerList.removeFirst();
        return CN_OK;
    }
    return CN_INVALID;
}