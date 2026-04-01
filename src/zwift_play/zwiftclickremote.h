#ifndef ZWIFTCLICKREMOTE_H
#define ZWIFTCLICKREMOTE_H

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
#include "zwiftPlayDevice.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class zwiftclickremote : public bluetoothdevice {
    Q_OBJECT
  public:

    zwiftclickremote(bluetoothdevice *parentDevice, AbstractZapDevice::ZWIFT_PLAY_TYPE typeZap);
    bool connected() override;
    ZwiftPlayDevice* playDevice = new ZwiftPlayDevice();
    void vibrate(uint8_t pattern);
    AbstractZapDevice::ZWIFT_PLAY_TYPE typeZap = AbstractZapDevice::NONE;
    
    // Wrapper per characteristicChanged che accetta direttamente QBluetoothUuid
    void handleCharacteristicValueChanged(const QBluetoothUuid &uuid, const QByteArray &newValue);

  private:
    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattNotify1Characteristic;
    QLowEnergyCharacteristic gattNotify2Characteristic;
    QLowEnergyCharacteristic gattWrite1Characteristic;
    QLowEnergyService *gattWrite1Service;

    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic *writeChar, uint8_t *data,
                             uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);

    bluetoothdevice *parentDevice = nullptr;

    bool initDone = false;
    bool initRequest = false;

    QTimer *refresh;

    uint32_t countRxTimeout = 0;

#ifdef Q_OS_IOS
    lockscreen* iOS_zwiftClickRemote = nullptr;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

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

#endif // ZWIFTCLICKREMOTE_H
