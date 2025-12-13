#ifndef CYCPLUSBC2_H
#define CYCPLUSBC2_H

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

class cycplusbc2 : public bluetoothdevice {
    Q_OBJECT
  public:

    cycplusbc2(bluetoothdevice *parentDevice);
    bool connected() override;

    // Wrapper per characteristicChanged che accetta direttamente QBluetoothUuid
    void handleCharacteristicValueChanged(const QBluetoothUuid &uuid, const QByteArray &newValue);

  private:
    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattNotifyCharacteristic;  // TX characteristic (device -> app)
    QLowEnergyCharacteristic gattWriteCharacteristic;   // RX characteristic (app -> device, unused)
    QLowEnergyService *gattService;

    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic *writeChar, uint8_t *data,
                             uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);

    bluetoothdevice *parentDevice = nullptr;

    bool initDone = false;
    bool initRequest = false;

    QTimer *refresh;

    uint32_t countRxTimeout = 0;

    // State tracking for shift buttons
    uint8_t lastShiftUpState = 0x00;
    uint8_t lastShiftDownState = 0x00;

#ifdef Q_OS_IOS
    lockscreen* iOS_cycplusbc2 = nullptr;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
    void plus();   // Shift up
    void minus();  // Shift down

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

#endif // CYCPLUSBC2_H
