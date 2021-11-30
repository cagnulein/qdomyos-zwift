#ifndef CHARACTERISTICNOTIFIER_H
#define CHARACTERISTICNOTIFIER_H

#include <QObject>

#define CN_INVALID -1
#define CN_OK 0

class CharacteristicNotifier : public QObject {
    Q_OBJECT
  public:
    explicit CharacteristicNotifier(QObject *parent = nullptr) : QObject(parent) {}
    virtual int notify(QByteArray &out) = 0;
  signals:
};

#endif // CHARACTERISTICNOTIFIER_H
