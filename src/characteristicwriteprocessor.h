#ifndef CHARACTERISTICWRITEPROCESSOR_H
#define CHARACTERISTICWRITEPROCESSOR_H

#include <QObject>

#define CP_INVALID -1
#define CP_OK 0

class CharacteristicWriteProcessor : public QObject {
    Q_OBJECT
  public:
    explicit CharacteristicWriteProcessor(QObject *parent = nullptr) : QObject(parent) {}
    virtual int writeProcess(quint16 uuid, const QByteArray &data, QByteArray &out) = 0;
  signals:
};

#endif // CHARACTERISTICWRITEPROCESSOR_H
