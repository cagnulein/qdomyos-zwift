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
#include "domyoselliptical.h"
#include "trxappgateusbtreadmill.h"
#include "trxappgateusbbike.h"
#include "toorxtreadmill.h"
#include "echelonconnectsport.h"
#include "schwinnic4bike.h"
#include "yesoulbike.h"
#include "flywheelbike.h"
#include "fassitreadmill.h"
#include "jkfitnesstreadmill.h"
#include "inspirebike.h"
#include "horizontreadmill.h"
#include "eslinkertreadmill.h"
#include "proformbike.h"
#include "proformtreadmill.h"
#include "sportstechbike.h"
#include "heartratebelt.h"
#include "bluetoothdevice.h"
#include "signalhandler.h"

class bluetooth : public QObject, public SignalHandler
{
    Q_OBJECT
public:
    explicit bluetooth(bool logs, QString deviceName = "", bool noWriteResistance = false, bool noHeartService = false, uint32_t pollDeviceTime = 200, bool noConsole = false, bool testResistance = false, uint8_t bikeResistanceOffset = 4, uint8_t bikeResistanceGain = 1);
    bluetoothdevice* device();
    QList<QBluetoothDeviceInfo> devices;
    bool onlyDiscover = false;

private:
    QFile* debugCommsLog = 0;
    QBluetoothDeviceDiscoveryAgent *discoveryAgent;
    fassitreadmill* fassiTreadmill = 0;
    jkfitnesstreadmill* jkfitnessTreadmill = 0;
    domyostreadmill* domyos = 0;
    domyosbike* domyosBike = 0;
    domyoselliptical* domyosElliptical = 0;
    toorxtreadmill* toorx = 0;
    trxappgateusbtreadmill* trxappgateusb = 0;
    trxappgateusbbike* trxappgateusbBike = 0;
    echelonconnectsport* echelonConnectSport = 0;
    yesoulbike* yesoulBike = 0;
    flywheelbike* flywheelBike = 0;
    proformbike* proformBike = 0;
    proformtreadmill* proformTreadmill = 0;
    horizontreadmill* horizonTreadmill = 0;
    schwinnic4bike* schwinnIC4Bike = 0;
    sportstechbike* sportsTechBike = 0;
    inspirebike* inspireBike = 0;
    eslinkertreadmill* eslinkerTreadmill = 0;
    heartratebelt* heartRateBelt = 0;
    QString filterDevice = "";
    bool testResistance = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noConsole = false;
    bool logs = true;
    uint32_t pollDeviceTime = 200;
    uint8_t bikeResistanceOffset = 4;
    uint8_t bikeResistanceGain = 1;
    bool forceHeartBeltOffForTimeout = false;

    bool handleSignal(int signal);
    void stateFileUpdate();
    void stateFileRead();
    bool heartRateBeltAvaiable();

signals:
    void deviceConnected();
    void deviceFound(QString name);
    void searchingStop();

public slots:
    void restart();
    void debug(QString string);
    void heartRate(uint8_t heart);

private slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    void deviceUpdated(const QBluetoothDeviceInfo &device, QBluetoothDeviceInfo::Fields updateFields);
#endif
    void canceled();
    void finished();
    void speedChanged(double);
    void inclinationChanged(double);
    void connectedAndDiscovered();

signals:

};

#endif // BLUETOOTH_H
