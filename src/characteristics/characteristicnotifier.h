#ifndef CHARACTERISTICNOTIFIER_H
#define CHARACTERISTICNOTIFIER_H

#include <QObject>

#define CN_INVALID -1
#define CN_OK 0

class bluetoothdevice;

class CharacteristicNotifier : public QObject {
    Q_OBJECT
    quint16 my_uuid;

  protected:
    // The device every subclass reads its values from. Held in the base so that
    // DirconManager::setDevice() can rebind the whole notifier chain in one pass;
    // subclasses that used to declare their own `Bike` now inherit this one.
    bluetoothdevice *Bike = nullptr;

  public:
    explicit CharacteristicNotifier(quint16 uuid, bluetoothdevice *device, QObject *parent = nullptr)
        : QObject(parent), my_uuid(uuid), Bike(device) {}
    virtual int notify(QByteArray &out) = 0;
    quint16 uuid() const { return my_uuid; }

    // May be called with nullptr: notify() implementations must tolerate a null
    // device once the DIRCON endpoint outlives the bike.
    virtual void setDevice(bluetoothdevice *device) { Bike = device; }
    bluetoothdevice *device() const { return Bike; }
  signals:
};

#endif // CHARACTERISTICNOTIFIER_H
