#ifndef TECHNOGYMMYRUNTREADMILLRFCOMM_H
#define TECHNOGYMMYRUNTREADMILLRFCOMM_H

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

class technogymmyruntreadmillrfcomm : public treadmill {
    Q_OBJECT
  public:
    explicit technogymmyruntreadmillrfcomm();
    bool canHandleSpeedChange() override { return false; }
    bool canHandleInclineChange() override { return false; }    

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
    void changeInclinationRequested(double grade, double percentage);

  private:
    QBluetoothServiceDiscoveryAgent *discoveryAgent;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;


    QTimer *refresh;
    bool initDone = false;
    volatile bool found = false;

    void waitForAPacket();

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
};

#endif // TECHNOGYMMYRUNTREADMILLRFCOMM_H
