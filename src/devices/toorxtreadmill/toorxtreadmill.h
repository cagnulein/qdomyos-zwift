#ifndef TOORX_H
#define TOORX_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>
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
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>

#include "treadmill.h"

class toorxtreadmill : public treadmill {
    Q_OBJECT
  public:
    explicit toorxtreadmill();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void serviceCanceled(void);
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void serviceFinished();
    void readSocket();
    void rfCommConnected();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError);
    void update();

  private:
    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;

    QTimer *refresh;
    bool initDone = false;
    bool found = false;

    void send(char * buffer, int size);

    bool MASTERT409 = false;

    uint16_t GetElapsedTimeFromPacket(const QByteArray &packet);
    double GetDistanceFromPacket(const QByteArray &packet);
    uint16_t GetCaloriesFromPacket(const QByteArray &packet);
    double GetSpeedFromPacket(const QByteArray &packet);
    uint8_t GetInclinationFromPacket(const QByteArray &packet);
    uint8_t GetHeartRateFromPacket(const QByteArray &packet);

  signals:
    void disconnected();
    void debug(QString string);
};

#endif // TOORX_H
