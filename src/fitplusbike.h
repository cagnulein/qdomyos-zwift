#ifndef FITPLUSBIKE_H
#define FITPLUSBIKE_H

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

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class fitplusbike : public bike
{
    Q_OBJECT
public:
    fitplusbike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset, double bikeResistanceGain);
    int pelotonToBikeResistance(int pelotonResistance);
    uint8_t resistanceFromPowerRequest(uint16_t power);
    bool connected();

    void* VirtualBike();
    void* VirtualDevice();

private:
    const int max_resistance = 32;
    double bikeResistanceToPeloton(double resistance);
    double GetDistanceFromPacket(QByteArray packet);
    uint16_t wattsFromResistance(double resistance);
    QTime GetElapsedFromPacket(QByteArray packet);
    void btinit();
    void writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log=false,  bool wait_for_response = false);
    void startDiscover();
    void forceResistance(int8_t requestResistance);
    void sendPoll();
    uint16_t watts();

    QTimer* refresh;
    virtualbike* virtualBike = 0;

    QLowEnergyService* gattCommunicationChannelService = 0;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    uint8_t counterPoll = 1;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    int8_t lastResistanceBeforeDisconnection = -1;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

#ifdef Q_OS_IOS
    lockscreen* h = 0;
#endif

signals:
    void disconnected();

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

#endif // FITPLUSBIKE_H
