#ifndef ICONSOLEBIKE_H
#define ICONSOLEBIKE_H

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

class iconsolebike : public bike {
    Q_OBJECT
  public:
    explicit iconsolebike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                          double bikeResistanceGain);
    ~iconsolebike();

    bool connected() override;

    resistance_t pelotonToBikeResistance(int pelotonResistance) override;

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
    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;

    QTimer *refresh = nullptr;
    bool initDone = false;
    bool found = false;

    uint8_t sec1update = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    // Protocol state
    bool noWriteResistance = false;
    bool noHeartService = false;
    int initPhase = 0;

    // Timing for metric calculations
    QDateTime lastRefreshCharacteristicChanged;
    uint64_t CrankRevs = 0;
    uint16_t LastCrankEventTime = 0;

    void send(const uint8_t *buffer, int size);
    void btinit();
    void forceResistance(resistance_t requestResistance);

    // Packet parsing
    uint16_t GetCadenceFromPacket(const QByteArray &packet);
    double GetSpeedFromPacket(const QByteArray &packet);
    uint16_t GetPowerFromPacket(const QByteArray &packet);
    uint8_t GetResistanceFromPacket(const QByteArray &packet);

    // Resistance conversions
    double bikeResistanceToPeloton(double bikeResistance);

  signals:
    void disconnected();
    void debug(QString string);
};

#endif // ICONSOLEBIKE_H
