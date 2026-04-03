#ifndef STAIRMASTER8G_H
#define STAIRMASTER8G_H

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
#include <QtCore/qqueue.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>
#include <QString>

#include "stairclimber.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class stairmaster8g : public stairclimber {
    Q_OBJECT
  public:
    stairmaster8g(uint32_t pollDeviceTime = 200, bool noConsole = false, bool noHeartService = false,
                  double forceInitSpeed = 0.0, double forceInitInclination = 0.0);
    bool connected() override;

  private:
    void btinit();
    void processPrimaryFrame(const QByteArray &frame);
    void processSecondaryFrame(const QByteArray &frame);
    void startDiscover();

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattCommandCharacteristic;
    QLowEnergyCharacteristic gattDataCharacteristic;

    QTimer *refresh;
    bool initDone = false;
    bool initRequest = false;
    bool noConsole = false;
    bool noHeartService = false;
    uint32_t pollDeviceTime = 200;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;
    QQueue<QPair<qint64, int>> cadenceSamples;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void speedChanged(double speed);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void changeInclinationRequested(double grade, double percentage);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // STAIRMASTER8G_H
