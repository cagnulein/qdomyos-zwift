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
#include "devices/bluetoothdevice.h"
#include "virtualdevice.h"

class virtualrower : public virtualdevice {

    Q_OBJECT
  public:
    virtualrower(bluetoothdevice *t, bool noWriteResistance = false, bool noHeartService = false);
    bool connected() override;

  private:
    QLowEnergyController *leController = nullptr;
    QLowEnergyService *serviceHR = nullptr;
    QLowEnergyService *serviceBattery = nullptr;
    QLowEnergyService *serviceFIT = nullptr;
    QLowEnergyService *servicePM5Rowing = nullptr;
    QLowEnergyService *servicePM5DeviceInfo = nullptr;
    QLowEnergyService *servicePM5GAP = nullptr;
    QLowEnergyService *servicePM5Control = nullptr;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataFIT;
    QLowEnergyServiceData serviceDataPM5Rowing;
    QLowEnergyServiceData serviceDataPM5DeviceInfo;
    QLowEnergyServiceData serviceDataPM5GAP;
    QLowEnergyServiceData serviceDataPM5Control;
    QTimer rowerTimer;
    bluetoothdevice *Rower;

    bool pm5Mode = false;

    uint16_t lastWheelTime = 0;
    uint32_t wheelRevs = 0;
    qint64 lastFTMSFrameReceived = 0;

    bool noHeartService = false;

    void writeCharacteristic(QLowEnergyService *service, const QLowEnergyCharacteristic &characteristic,
                             const QByteArray &value);
    void setupPM5Services();
    QByteArray buildPM5GeneralStatus();
    QByteArray buildPM5AdditionalStatus();
    QByteArray buildPM5AdditionalStatus2();
    QByteArray buildPM5StrokeData();
    QByteArray buildPM5AdditionalStrokeData();

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
