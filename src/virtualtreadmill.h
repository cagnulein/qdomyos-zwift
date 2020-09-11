#ifndef VIRTUALTREADMILL_H
#define VIRTUALTREADMILL_H

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

class virtualtreadmill: QObject
{
    Q_OBJECT
public:
    virtualtreadmill();    

private:
    QLowEnergyController* leController;
    QLowEnergyService* service;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceData;
    QTimer treadmillTimer;

private slots:
    void treadmillProvider();
    void reconnect();
};

#endif // VIRTUALTREADMILL_H
