#ifndef DEERRUNTREADMILL_H
#define DEERRUNTREADMILL_H

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

#include <QDateTime>
#include <QObject>

#include "devices/treadmill.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class deerruntreadmill : public treadmill {

    Q_OBJECT
  public:
    deerruntreadmill(uint32_t poolDeviceTime = 200, bool noConsole = false, bool noHeartService = false,
                     double forceInitSpeed = 0.0, double forceInitInclination = 0.0);
    bool connected() override;
    double minStepInclination() override;

  private:
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    void btinit(bool startTape);
    void writeCharacteristic(const QLowEnergyCharacteristic characteristic, uint8_t *data, uint8_t data_len,
                             const QString &info, bool disable_log = false, bool wait_for_response = false);
    void writeUnlockCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false);
    void waitForAPacket();
    void startDiscover();
    uint8_t calculateXOR(uint8_t arr[], size_t size);
    uint8_t calculatePitPatChecksum(uint8_t arr[], size_t size);
    bool noConsole = false;
    bool noHeartService = false;
    uint32_t pollDeviceTime = 200;
    uint8_t pollCounter = 0;
    bool searchStopped = false;
    uint8_t sec1Update = 0;
    uint8_t firstInit = 0;
    QByteArray lastPacket;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;
    
    QLowEnergyService *unlock_service = nullptr;
    QLowEnergyCharacteristic unlock_characteristic;

    bool pitpat = false;
    bool superun_ba04 = false;

    bool initDone = false;
    bool initRequest = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  Q_SIGNALS:
    void disconnected();
    void debug(QString string);
    void speedChanged(double speed);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void searchingStop();

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

#endif // DEERRUNTREADMILL_H
