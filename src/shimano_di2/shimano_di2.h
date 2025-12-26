#ifndef SHIMANO_DI2_H
#define SHIMANO_DI2_H

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

#include "devices/bluetoothdevice.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class shimano_di2 : public bluetoothdevice {
    Q_OBJECT
  public:

    shimano_di2(bluetoothdevice *parentDevice);
    bool connected() override;

  private:
    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    bluetoothdevice *parentDevice = nullptr;

    bool initDone = false;
    bool initRequest = false;

    QTimer *refresh;

    // Store last button states for change detection
    QMap<int, uint8_t> lastButtons;

#ifdef Q_OS_IOS
    lockscreen* iOS_shimanoDi2 = nullptr;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
    void plus();
    void minus();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void disconnectBluetooth();
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

  private slots:
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // SHIMANO_DI2_H
