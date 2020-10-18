#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <QObject>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QtCore/qbytearray.h>
#include <QtCore/qloggingcategory.h>
#include <QFile>

#include "treadmill.h"
#include "domyostreadmill.h"
#include "domyosbike.h"
#include "toorxtreadmill.h"

class bluetooth : public QObject
{
    Q_OBJECT
public:
    explicit bluetooth(bool logs);
    treadmill* treadMill();

private:
    QFile* debugCommsLog = 0;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    domyostreadmill* domyos = 0;
    domyosbike* domyosBike = 0;
    toorxtreadmill* toorx = 0;

public slots:
    void restart();
    void debug(QString string);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

signals:

};

#endif // BLUETOOTH_H
