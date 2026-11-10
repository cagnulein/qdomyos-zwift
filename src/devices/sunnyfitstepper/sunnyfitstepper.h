#ifndef SUNNYFITSTEPPER_H
#define SUNNYFITSTEPPER_H

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
#include <QString>

#include "stairclimber.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class sunnyfitstepper : public stairclimber {
    Q_OBJECT
  public:
    sunnyfitstepper(uint32_t pollDeviceTime = 200, bool noConsole = false, bool noHeartService = false,
                    double forceInitSpeed = 0.0, double forceInitInclination = 0.0);
    bool connected() override;

  private:
    void btinit();
    void sendPoll();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void processDataFrame(const QByteArray &completeFrame);
    void startDiscover();

    // Bluetooth
    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;
    QLowEnergyCharacteristic gattNotify4Characteristic;

    // Split-frame handling (CRITICAL)
    QByteArray frameBuffer;
    bool expectingSecondPart = false;

    // State
    QTimer *refresh;
    uint8_t sec1Update = 0;
    uint8_t counterPoll = 0;
    bool initDone = false;
    bool initRequest = false;
    bool noConsole = false;
    bool noHeartService = false;
    uint32_t pollDeviceTime = 200;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;

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

#endif // SUNNYFITSTEPPER_H
