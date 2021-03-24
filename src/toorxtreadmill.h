#ifndef TOORX_H
#define TOORX_H

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>
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

#include "virtualtreadmill.h"
#include "treadmill.h"

class toorxtreadmill : public treadmill
{
    Q_OBJECT
public:
    explicit toorxtreadmill();

public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void readSocket();
    void rfCommConnected();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError);
    void update();

private:
    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;

    virtualtreadmill* virtualTreadMill = 0;

    QTimer* refresh;
    bool initDone = false;

    uint16_t GetElapsedTimeFromPacket(QByteArray packet);
    uint16_t GetDistanceFromPacket(QByteArray packet);
    uint16_t GetCaloriesFromPacket(QByteArray packet);
    double GetSpeedFromPacket(QByteArray packet);
    uint8_t GetInclinationFromPacket(QByteArray packet);
    uint8_t GetHeartRateFromPacket(QByteArray packet);

signals:
    void disconnected();
    void debug(QString string);

};

#endif // TOORX_H
