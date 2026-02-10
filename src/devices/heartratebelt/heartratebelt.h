#ifndef HEARTRATEBELT_H
#define HEARTRATEBELT_H

#include <QBluetoothDeviceDiscoveryAgent>
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

#include <QObject>
#include <QTime>
#include <QTimer>
#include <QDateTime>

#include "treadmill.h"

class heartratebelt : public treadmill {
    Q_OBJECT
  public:
    heartratebelt();
    ~heartratebelt();
    bool connected() override;

  private:
    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyService *gattBatteryService = nullptr;
    QLowEnergyCharacteristic gattNotifyCharacteristic;
    QDateTime connectingTime;  // Timestamp when entering connecting state
    static const int CONNECTION_TIMEOUT = 10000; // 10 seconds in milliseconds
    QTimer* updateTimer;  // Timer for periodic updates
    uint8_t battery_level = 0;

    QTimer *refresh;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
    void heartRate(uint8_t heart) override;

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void disconnectBluetooth();

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // HEARTRATEBELT_H