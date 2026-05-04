#ifndef CHARACTERISTICNOTIFIER_H
#define CHARACTERISTICNOTIFIER_H

#include <QObject>

#define CN_INVALID -1
#define CN_OK 0

class CharacteristicNotifier : public QObject {
    Q_OBJECT
    quint16 my_uuid;

  public:
    explicit CharacteristicNotifier(quint16 uuid, QObject *parent = nullptr) : QObject(parent), my_uuid(uuid) {}
    virtual int notify(QByteArray &out) = 0;
    quint16 uuid() const { return my_uuid; }
  signals:
};

#endif // CHARACTERISTICNOTIFIER_H
