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

#include "bike.h"

class virtualbike: public QObject
{
    Q_OBJECT
public:
    virtualbike(bike* t, bool noWriteResistance = false, bool noHeartService = false, uint8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    bool connected();

private:
    QLowEnergyController* leController;
    QLowEnergyService* serviceHR;
    QLowEnergyService* serviceBattery;
    QLowEnergyService* serviceFIT;
    QLowEnergyService* service;
    QLowEnergyService* serviceChanged;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataBattery;
    QLowEnergyServiceData serviceDataFIT;
    QLowEnergyServiceData serviceData;
    QLowEnergyServiceData serviceDataChanged;
    QLowEnergyServiceData serviceEchelon;
    QTimer bikeTimer;
    bike* Bike;

    uint16_t lastWheelTime = 0;
    uint32_t wheelRevs = 0;

    bool noHeartService = false;
    uint8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    void writeCharacteristic(QLowEnergyService* service, QLowEnergyCharacteristic characteristic, QByteArray value);

private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void bikeProvider();
    void reconnect();
    void error(QLowEnergyController::Error newError);
};

#endif // VIRTUALBIKE_H
