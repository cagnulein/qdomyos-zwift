#ifndef VIRTUALBIKE_H
#define VIRTUALBIKE_H

#include <QObject>

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>

#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qbytearray.h>
#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif
#include "bike.h"
#include "dirconmanager.h"

class virtualbike : public QObject {

    Q_OBJECT
  public:
    virtualbike(bluetoothdevice *t, bool noWriteResistance = false, bool noHeartService = false,
                uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    bool connected();
    bool ftmsDeviceConnected() {return lastFTMSFrameReceived != 0;}

  private:
    QLowEnergyController *leController = nullptr;
    QLowEnergyService *serviceHR = nullptr;
    QLowEnergyService *serviceBattery = nullptr;
    QLowEnergyService *serviceFIT = nullptr;
    QLowEnergyService *service = nullptr;
    QLowEnergyService *serviceChanged = nullptr;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataBattery;
    QLowEnergyServiceData serviceDataFIT;
    QLowEnergyServiceData serviceData;
    QLowEnergyServiceData serviceDataChanged;
    QLowEnergyServiceData serviceEchelon;
    QTimer bikeTimer;
    bluetoothdevice *Bike;
    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicNotifier2AD2 *notif2AD2 = 0;
    CharacteristicNotifier2A63 *notif2A63 = 0;
    CharacteristicNotifier2A37 *notif2A37 = 0;
    CharacteristicNotifier2A5B *notif2A5B = 0;

    qint64 lastFTMSFrameReceived = 0;

    bool noHeartService = false;
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    DirconManager *dirconManager = 0;
    int iFit_pelotonToBikeResistance(int pelotonResistance);

    bool echelonInitDone = false;
    void echelonWriteResistance();

    void writeCharacteristic(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic,
                             const QByteArray &value);

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void changeInclination(double grade, double percentage);

    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void bikeProvider();
    void reconnect();
    void error(QLowEnergyController::Error newError);
};

#endif // VIRTUALBIKE_H
