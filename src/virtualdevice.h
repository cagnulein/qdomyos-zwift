#ifndef VIRTUALDEVICE_H
#define VIRTUALDEVICE_H

#include <QObject>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include "lockscreen/qzlockscreenfunctions.h"

class virtualdevice : public QObject
{
    Q_OBJECT
private:
    QZLockscreenFunctions * lockscreenFunctions = nullptr;
protected:
    QZLockscreenFunctions * getLockscreenFunctions() const;
signals:
    void debug(QString string);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
public:
    explicit virtualdevice(QObject *parent = nullptr);
    ~virtualdevice() override;
    virtual bool connected()=0;



};

#endif // VIRTUALDEVICE_H
