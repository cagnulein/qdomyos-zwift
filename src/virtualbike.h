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

class virtualbike: QObject
{
    Q_OBJECT
public:
    virtualbike(bike* t);
    bool connected();

private:
    QLowEnergyController* leController;
    QLowEnergyService* service;
    QLowEnergyService* serviceHR;
    QLowEnergyService* serviceFIT;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceData;
    QLowEnergyServiceData serviceDataHR;
    QLowEnergyServiceData serviceDataFIT;
    QTimer bikeTimer;
    bike* Bike;

private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void bikeProvider();
    void reconnect();
};

#endif // VIRTUALBIKE_H
