#ifndef KETTLERRUN7TREADMILL_H
#define KETTLERRUN7TREADMILL_H

#include <QByteArray>
#include <QQueue>
#include <QTimer>
#include <QtBluetooth/QLowEnergyCharacteristic>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyDescriptor>
#include <QtBluetooth/QLowEnergyService>

#include "devices/kettlerclassicbike/kettlerclassicprotocol.h"
#include "devices/treadmill.h"

// BLE/ISSC transport for the Kettler RUN 7 IRMP protocol. The IRMP codec is
// shared with Kettler Classic, but no RFCOMM/Classic discovery is used here.
// Experimental read-only implementation; no motor/start/stop writes.
class kettlerrun7treadmill : public treadmill {
    Q_OBJECT

  public:
    kettlerrun7treadmill();
    bool connected() override;
    bool canStartStop() override { return false; }
    bool canHandleSpeedChange() override { return false; }
    bool canHandleInclineChange() override { return false; }

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  signals:
    void debug(QString message);
    void disconnected();

  private slots:
    void serviceScanDone();
    void serviceStateChanged(QLowEnergyService::ServiceState state);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void controllerDisconnected();
    void requestTimedOut();
    void update();

  private:
    static constexpr quint16 Handshake = 0x0001;
    static constexpr quint16 Pulse = 0x0008;
    static constexpr quint16 DeviceName = 0x0014;
    static constexpr quint16 DeviceType = 0x0018;
    static constexpr quint16 InclineCurrent = 0x001f;
    static constexpr quint16 SpeedCurrent = 0x0023;
    static constexpr quint8 Read = 0x01;
    static constexpr quint8 Answer = 0x03;
    static constexpr quint8 Status = 0x04;
    static constexpr quint8 Error = 0x05;

    void enqueueRead(quint16 function);
    void sendNextRequest();
    void sendNextChunk();
    void chunkWritten();
    void handleFrame(const kettlerclassicprotocol::Frame &frame);
    void fail(const QString &reason);
    static bool validHandshakeResponse(const QByteArray &payload);
    static quint16 be16(const QByteArray &payload);

    QLowEnergyService *service = nullptr;
    QLowEnergyCharacteristic txCharacteristic;
    QLowEnergyCharacteristic rxCharacteristic;
    QByteArray receiveBuffer;
    QByteArray pendingWrite;
    QQueue<quint16> requests;
    QTimer requestTimer;
    QTimer refresh;
    quint16 activeFunction = 0;
    bool requestInFlight = false;
    bool waitingForWrite = false;
    bool writeWithResponse = true;
    bool handshaken = false;
    bool ready = false;
    bool disconnectedReported = false;
    int telemetryTimeouts = 0;
};

#endif // KETTLERRUN7TREADMILL_H
