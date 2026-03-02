#ifndef ICONCEPTBIKE_H
#define ICONCEPTBIKE_H

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

#include "devices/bike.h"

class iconceptbike : public bike {
    Q_OBJECT
  public:
    explicit iconceptbike();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
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
    uint8_t firstStateChanged = 0;
    bool i_Nexor = false;

    uint16_t GetElapsedTimeFromPacket(const QByteArray &packet);
    uint16_t GetDistanceFromPacket(const QByteArray &packet);
    uint16_t GetCaloriesFromPacket(const QByteArray &packet);
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetWattFromPacket(const QByteArray &packet);

    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    uint16_t watts() override;
    
#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
};

#endif // ICONCEPTBIKE_H
