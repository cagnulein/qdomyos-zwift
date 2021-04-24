#ifndef TRXAPPGATEUSBBIKE_H
#define TRXAPPGATEUSBBIKE_H


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

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>
#include <QtCore/qmutex.h>

#include <QObject>
#include <QTime>

#include "virtualbike.h"
#include "bike.h"

class trxappgateusbbike : public bike
{
    Q_OBJECT
public:
    trxappgateusbbike(bool noWriteResistance, bool noHeartService);
    bool connected();
    double odometer();

    void* VirtualBike();
    void* VirtualDevice();

private:
    double GetSpeedFromPacket(QByteArray packet);
    double GetResistanceFromPacket(QByteArray packet);
    double GetKcalFromPacket(QByteArray packet);
    double GetDistanceFromPacket(QByteArray packet);
    uint16_t GetElapsedFromPacket(QByteArray packet);
    void forceResistance(int8_t requestResistance);
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    void writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log, bool wait_for_response);
    void startDiscover();
    uint16_t watts();
    double GetWattFromPacket(QByteArray packet);
    double GetCadenceFromPacket(QByteArray packet);
    double DistanceCalculated = 0;

    QTimer* refresh;
    virtualbike* virtualBike = 0;

    bool noWriteResistance = false;
    bool noHeartService = false;

    uint8_t firstVirtualBike = 0;
    bool firstCharChanged = true;
    QTime lastTimeCharChanged;
    uint8_t sec1update = 0;
    QByteArray lastPacket;

    QLowEnergyService* gattCommunicationChannelService = 0;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;
    QLowEnergyCharacteristic gattNotify2Characteristic;

    bool initDone = false;
    bool initRequest = false;
    bool readyToStart = false;

    typedef enum TYPE {
        TRXAPPGATE = 0,
        IRUNNING = 1,
        CHANGYOW = 2,
        SKANDIKAWIRY = 3,
        ICONSOLE = 4,
        JLL_IC400 = 5,
        DKN_MOTION = 6,
    } TYPE;
    TYPE bike_type = TRXAPPGATE;


signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // TRXAPPGATEUSBBIKE_H
