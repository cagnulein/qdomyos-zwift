#ifndef THINKRIDERCONTROLLER_H
#define THINKRIDERCONTROLLER_H

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

class thinkridercontroller : public bluetoothdevice {
    Q_OBJECT
  public:
    thinkridercontroller(bluetoothdevice *parentDevice);
    bool connected() override;

  private:
    // Thinkrider VS200 UUIDs
    static const QBluetoothUuid SERVICE_UUID;
    static const QBluetoothUuid CHARACTERISTIC_UUID;

    // Button patterns
    static const QByteArray SHIFT_UP_PATTERN;
    static const QByteArray SHIFT_DOWN_PATTERN;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    bluetoothdevice *parentDevice = nullptr;

    bool initDone = false;

  signals:
    void disconnected();
    void debug(QString string);
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
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // THINKRIDERCONTROLLER_H
