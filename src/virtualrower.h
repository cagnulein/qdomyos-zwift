#ifndef VIRTUALROWER_H
#define VIRTUALROWER_H

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

class virtualrower : public QObject {

    Q_OBJECT
  public:
    virtualrower(bluetoothdevice *t, bool noWriteResistance = false, bool noHeartService = false);
    bool connected();

  private:
    QLowEnergyController *leController = nullptr;
    QLowEnergyService *serviceHR = nullptr;
    QLowEnergyService *serviceBattery = nullptr;
    QLowEnergyService *serviceFIT = nullptr;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataFIT;
    QTimer rowerTimer;
    bluetoothdevice *Rower;

    uint16_t lastWheelTime = 0;
    uint32_t wheelRevs = 0;
    qint64 lastFTMSFrameReceived = 0;

    bool noHeartService = false;

    void writeCharacteristic(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic,
                             const QByteArray &value);

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    
  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void rowerProvider();
    void reconnect();
    void error(QLowEnergyController::Error newError);
};

#endif // VIRTUALROWER_H
