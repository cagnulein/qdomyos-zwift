#ifndef UREVOTREADMILL_H
#define UREVOTREADMILL_H

#include "treadmill.h"
#include "urevoprotocol.h"

#include <QQueue>
#include <QTimer>
#include <QtBluetooth/QLowEnergyCharacteristic>
#include <QtBluetooth/QLowEnergyController>
#include <QtBluetooth/QLowEnergyDescriptor>
#include <QtBluetooth/QLowEnergyService>

class urevotreadmill : public treadmill {
    Q_OBJECT
public:
    explicit urevotreadmill(bool noHeartService = false);
    bool connected() override;
    double minStepSpeed() override { return 0.1609344; } // 0.1 mph
    double minStepInclination() override { return 0.1; }
    bool autoPauseWhenSpeedIsZero() override { return true; }
    bool autoStartWhenSpeedIsGreaterThenZero() override { return true; }

signals:
    void debug(QString message);
    void disconnected();

public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

private slots:
    void serviceScanDone();
    void serviceStateChanged(QLowEnergyService::ServiceState state);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void update();

private:
    void tryConfigure();
    void collectProtocolCommands();
    void flushWriteQueue();
    void handleFullStatus(qint64 now);
    void createVirtualTreadmill();
    void resetConnectionState();

    QTimer *m_refresh = nullptr;
    QLowEnergyService *m_mainService = nullptr; // FFF0
    QLowEnergyService *m_auxService = nullptr;  // FEE0
    QLowEnergyService *m_ftmsService = nullptr; // 1826: telemetry only
    QLowEnergyCharacteristic m_mainWrite;      // FFF2
    QLowEnergyCharacteristic m_mainNotify;     // FFF1
    QLowEnergyCharacteristic m_auxWrite;       // FEE2
    QLowEnergyCharacteristic m_auxNotify;      // FEE1
    QLowEnergyCharacteristic m_ftmsData;       // 2ACD
    QLowEnergyDescriptor m_mainCccd;
    QLowEnergyDescriptor m_auxCccd;

    UrevoProtocol m_protocol;
    QQueue<UrevoProtocol::Command> m_writeQueue;
    bool m_configured = false;
    bool m_mainSubscribed = false;
    bool m_auxSubscribed = false;
    bool m_connectedSignalSent = false;
    bool m_writeInFlight = false;
    bool m_noHeartService = false;
    bool m_firstVirtual = false;
    bool m_inFlightStop = false;
    qint64 m_writeAt = 0;
    qint64 m_lastFullAt = 0;
    qint64 m_lastStartRequestAt = 0;
    qint64 m_pendingTargetAt = 0;
    double m_pendingSpeed = -1;
    double m_pendingInclination = -100;
};
#endif // UREVOTREADMILL_H
