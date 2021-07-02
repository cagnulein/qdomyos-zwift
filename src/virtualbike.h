#ifndef VIRTUALBIKE_H
#define VIRTUALBIKE_H

#include <QObject>

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
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

class virtualbike: public QObject
{
    Q_OBJECT
public:
    virtualbike(bluetoothdevice* t, bool noWriteResistance = false, bool noHeartService = false, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    bool connected();

private:
    QLowEnergyController* leController = 0;
    QLowEnergyService* serviceHR = 0;
    QLowEnergyService* serviceBattery = 0;
    QLowEnergyService* serviceFIT = 0;
    QLowEnergyService* service = 0;
    QLowEnergyService* serviceChanged = 0;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataBattery;
    QLowEnergyServiceData serviceDataFIT;
    QLowEnergyServiceData serviceData;
    QLowEnergyServiceData serviceDataChanged;
    QLowEnergyServiceData serviceEchelon;
    QTimer bikeTimer;
    bluetoothdevice* Bike;

    uint16_t lastWheelTime = 0;
    uint32_t wheelRevs = 0;

    bool noHeartService = false;
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    void writeCharacteristic(QLowEnergyService* service, QLowEnergyCharacteristic characteristic, QByteArray value);
    
    void slopeChanged(int16_t slope);
    void powerChanged(uint16_t power);
    
#ifdef Q_OS_IOS
    lockscreen* h = 0;
#endif

signals:
    void changeInclination(double grade, double percentage);

private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void bikeProvider();
    void reconnect();
    void error(QLowEnergyController::Error newError);
};

#endif // VIRTUALBIKE_H
