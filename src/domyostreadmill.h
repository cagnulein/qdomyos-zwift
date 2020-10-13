#ifndef DOMYOSTREADMILL_H
#define DOMYOSTREADMILL_H

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
#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>
#include <QtCore/qmutex.h>

#include <QObject>

#include "virtualtreadmill.h"
#include "treadmill.h"

class domyostreadmill : public treadmill
{
    Q_OBJECT
public:
    domyostreadmill(bool logs = true);
    virtualtreadmill* virtualTreadMill = 0;
    bool connected();
    bool changeFanSpeed(uint8_t speed);

private:
    double GetSpeedFromPacket(QByteArray packet);
    double GetInclinationFromPacket(QByteArray packet);
    double GetKcalFromPacket(QByteArray packet);
    double GetDistanceFromPacket(QByteArray packet);    
    void forceSpeedOrIncline(double requestSpeed, double requestIncline);
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    void writeCharacteristic(uint8_t* data, uint8_t data_len, QString info, bool disable_log=false);
    void debug(QString text);
    void startDiscover();

    QTimer* refresh;

private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // DOMYOSTREADMILL_H
