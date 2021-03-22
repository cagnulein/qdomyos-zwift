#ifndef DOMYOSBIKE_H
#define DOMYOSBIKE_H

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
#include <QString>
#include <QDateTime>

#include "virtualbike.h"
#include "bike.h"

class domyosbike : public bike
{
    Q_OBJECT
public:
    domyosbike(bool noWriteResistance = false, bool noHeartService = false, bool testResistance = false, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    ~domyosbike();
    bool connected();

    void* VirtualBike();
    void* VirtualDevice();

private:
    double GetSpeedFromPacket(QByteArray packet);
    double GetInclinationFromPacket(QByteArray packet);
    double GetKcalFromPacket(QByteArray packet);
    double GetDistanceFromPacket(QByteArray packet);    
    void forceResistance(int8_t requestResistance);
    void updateDisplay(uint16_t elapsed);
    void btinit_changyow(bool startTape);
    void btinit_telink(bool startTape);
    void writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log=false,  bool wait_for_response = false);
    void startDiscover();
    uint16_t watts();

    QTimer* refresh;
    virtualbike* virtualBike = 0;
    uint8_t firstVirtual = 0;

    QLowEnergyService* gattCommunicationChannelService = 0;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    bool initDone = false;
    bool initRequest = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool testResistance = false;
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bool searchStopped = false;
    QDateTime lastTimeUpdate;
    bool firstUpdate = true;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    enum _BIKE_TYPE {
        CHANG_YOW,
        TELINK,
    };
    _BIKE_TYPE bike_type = CHANG_YOW;

signals:
    void disconnected();

public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void searchingStop();

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

#endif // DOMYOSBIKE_H
