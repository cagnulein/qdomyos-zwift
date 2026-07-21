#ifndef WAHOOKICKRUNTREADMILL_H
#define WAHOOKICKRUNTREADMILL_H

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

#include "treadmill.h"

// Wahoo KickRun treadmill - proprietary protocol
//
// Protocol summary (all commands/notifies on the "cmd" characteristic,
// which is the one having both Write-Without-Response and Notify):
//
// Speed command (app→device):
//   02 [b0 b1 b2] 00 [flag]
//   b0..b2 = uint24 LE, speed in μm/s (= round(km/h / 3.6 * 1e6))
//   flag   = 0xFF for first speed cmd of a workout session, 0x0A for subsequent
//
// Challenge-response (device→app, then app→device):
//   device sends: FD E0 01 [code_hi] [code_lo]  (after user presses paddle)
//   app replies:  E0 [code_hi] [code_lo]
//
// Telemetry (notify on the separate telemetry characteristic, ~2 Hz):
//   FF 01 [tgt_b0 tgt_b1 tgt_b2] 00 [cur_b0 cur_b1 cur_b2] ...
//   tgt = target speed (μm/s LE24); cur = current belt speed (μm/s LE24)

class wahookickruntreadmill : public treadmill {
    Q_OBJECT
  public:
    wahookickruntreadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;

  private:
    void writeCharacteristic(const QByteArray &data, const QString &info);
    void btinit();
    void forceSpeed(double speed);
    void startDiscover();

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool initDone = false;
    bool initRequest = false;
    bool firstSpeedSent = false;    // tracks whether to use 0xFF or 0x0A flag
    bool workoutModeStarted = false;

    QByteArray deviceId;            // 4-byte device ID from the 0x86 init response
    bool deviceIdReceived = false;

    QTimer *refresh;
    QList<QLowEnergyService *> gattServices;
    QLowEnergyService *gattCmdService = nullptr;    // service owning the cmd char
    QLowEnergyService *gattTelService = nullptr;    // service owning the telemetry char
    QLowEnergyCharacteristic gattCmdCharacteristic; // write+notify: handle 0x0023
    QLowEnergyCharacteristic gattTelCharacteristic; // notify only: handle 0x0026

    int notificationsSubscribed = 0;

    QByteArray lastPacket;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;

  Q_SIGNALS:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

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

#endif // WAHOOKICKRUNTREADMILL_H
