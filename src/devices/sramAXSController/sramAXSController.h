#ifndef SRAMAXSCONTROLLER_H
#define SRAMAXSCONTROLLER_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qqueue.h>
#include <QtCore/qtimer.h>

#include "devices/bluetoothdevice.h"

class sramaxscontroller : public bluetoothdevice {
    Q_OBJECT
  public:
    sramaxscontroller();
    bool connected() override;

    static bool isCompatibleDevice(const QBluetoothDeviceInfo &device);
    static int decodeRearGear(const QByteArray &plaintext);
    static int decodeRearGearCount(const QByteArray &plaintext);
    // QZ's virtual gear-up means a harder/larger virtual ratio. SRAM positions
    // increase toward the larger/easier cassette cogs, so the directions invert.
    static int virtualGearDirectionForRearDelta(int delta);

  private:
    static const QBluetoothUuid liveStateUuid();
    static const QBluetoothUuid liveStateChangedUuid();
    static const QBluetoothUuid drivetrainConfigUuid();
    static const QBluetoothUuid bondServiceUuid();
    static const QBluetoothUuid bondCharacteristicUuid();
    static const QBluetoothUuid serialUuid();

    QList<QLowEnergyService *> services;
    QLowEnergyService *liveService = nullptr;
    QLowEnergyService *bondService = nullptr;
    QLowEnergyService *serialService = nullptr;
    QLowEnergyService *configService = nullptr;
    QLowEnergyCharacteristic liveCharacteristic;
    QLowEnergyCharacteristic liveStateChangedCharacteristic;
    QLowEnergyCharacteristic drivetrainConfigCharacteristic;
    QLowEnergyCharacteristic bondCharacteristic;
    QLowEnergyCharacteristic serialCharacteristic;

    QByteArray deviceKey;
    QByteArray deviceIdentifier;
    QQueue<QByteArray> bondResponses;
    QTimer reconnectTimer;
    int lastRearGear = -1;
    int totalRearGears = 24;
    bool pairingStarted = false;
    bool readyStarted = false;

    void writeBondValue(const QByteArray &value);
    QByteArray waitForBondValue(int minimumLength, int timeoutMs = 5000, int exactLength = 0);
    void beginReady();
    void loadStoredKey();
    void startMonitoring();
    void startPairing();
    void readLiveState();
    void processLiveFrame(const QByteArray &frame);
    void processPlaintext(const QByteArray &plaintext);
    QString keySettingName() const;
    QString deviceIdentifierString() const;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
    void plus();
    void minus();
    void bondResponseReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void disconnectBluetooth();

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError err);
};

#endif // SRAMAXSCONTROLLER_H
