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
#include "trxappgateusbtreadmill.h"
#include "toorxtreadmill.h"
#include "bluetoothdevice.h"
#include "signalhandler.h"

class bluetooth : public QObject, public SignalHandler
{
    Q_OBJECT
public:
    explicit bluetooth(bool logs, QString deviceName = "", bool noWriteResistance = false, bool noHeartService = false, uint32_t pollDeviceTime = 200, bool noConsole = false, bool testResistance = false);
    bluetoothdevice* device();

private:
    QFile* debugCommsLog = 0;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    domyostreadmill* domyos = 0;
    domyosbike* domyosBike = 0;
    toorxtreadmill* toorx = 0;
    trxappgateusbtreadmill* trxappgateusb = 0;
    QString filterDevice = "";
    bool testResistance = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noConsole = false;
    bool logs = true;
    uint32_t pollDeviceTime = 200;

    bool handleSignal(int signal);
    void stateFileUpdate();

signals:
    void deviceConnected();
    void deviceFound(QString name);

public slots:
    void restart();
    void debug(QString string);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void speedChanged(double);
    void inclinationChanged(double);

signals:

};

#endif // BLUETOOTH_H
