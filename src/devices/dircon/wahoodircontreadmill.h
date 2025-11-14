#ifndef WAHOODIRCONTREADMILL_H
#define WAHOODIRCONTREADMILL_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif

#include "devices/treadmill.h"
#include "dirconpacket.h"
#include "dircondiscovery.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

/**
 * @brief Treadmill device that connects via Wahoo DirCon protocol over TCP
 *
 * This class implements a treadmill device that connects to Wahoo DirCon
 * servers over the network instead of Bluetooth.
 */
class wahoodircontreadmill : public treadmill {
    Q_OBJECT

public:
    /**
     * @brief Constructor for Wahoo DirCon Treadmill
     * @param deviceInfo Device information from mDNS discovery
     * @param noWriteResistance Disable resistance writes (not used for treadmill)
     * @param noHeartService Disable heart rate service
     */
    wahoodircontreadmill(const DirconDeviceInfo &deviceInfo, bool noWriteResistance, bool noHeartService);
    ~wahoodircontreadmill();

    // Override from bluetoothdevice
    bool connected() override;
    deviceType deviceType() const override { return TREADMILL; }

    // Override from treadmill
    void changeSpeed(double speed) override;
    void changeInclination(double inclination) override;
    double minStepSpeed() override { return 0.5; }
    double minStepInclination() override { return 0.5; }

private:
    // Device information
    DirconDeviceInfo deviceInfo;

    // TCP connection
    QTcpSocket *tcpSocket;
    bool isConnected;

    // DirCon protocol
    quint8 sequenceNumber;
    QByteArray receiveBuffer;

    // Timers
    QTimer *refresh;
    QTimer *connectionTimer;

    // State
    bool initDone;
    bool initRequest;
    bool servicesDiscovered;
    bool characteristicsDiscovered;
    QDateTime lastRefreshCharacteristicChanged;

    // Settings
    bool noWriteResistance;
    bool noHeartService;

    // Speed and inclination
    double lastSpeed;
    double lastInclination;

    // FTMS characteristic UUIDs
    static const quint16 UUID_FTMS_SERVICE = 0x1826;
    static const quint16 UUID_TREADMILL_DATA = 0x2ACD;
    static const quint16 UUID_HEART_RATE = 0x2A37;
    static const quint16 UUID_FITNESS_MACHINE_CONTROL_POINT = 0x2AD9;
    static const quint16 UUID_FITNESS_MACHINE_FEATURE = 0x2ACC;
    static const quint16 UUID_FITNESS_MACHINE_STATUS = 0x2ADA;

    // Methods
    void connectToDevice();
    void disconnectFromDevice();
    void sendDiscoverServices();
    void sendDiscoverCharacteristics();
    void sendSubscribeCharacteristic(quint16 uuid);
    void sendReadCharacteristic(quint16 uuid);
    void sendWriteCharacteristic(quint16 uuid, const QByteArray &data);

    bool writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
                           bool disable_log = false, bool wait_for_response = false);
    void startDiscover();
    void init();
    void forceSpeed(double speed);
    void forceInclination(double inclination);
    uint16_t watts() override;

    // FTMS protocol helpers
    void parseTreadmillData(const QByteArray &value);
    void parseHeartRate(const QByteArray &value);
    void parseFitnessMachineFeature(const QByteArray &value);
    void parseFitnessMachineStatus(const QByteArray &value);

    // DirCon packet handling
    void processDirconPacket(const DirconPacket &packet);
    void handleDiscoverServicesResponse(const DirconPacket &packet);
    void handleDiscoverCharacteristicsResponse(const DirconPacket &packet);
    void handleReadCharacteristicResponse(const DirconPacket &packet);
    void handleNotification(const DirconPacket &packet);

#ifdef Q_OS_IOS
    lockscreen *h = nullptr;
#endif

signals:
    void disconnected();
    void debug(QString string);

private slots:
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpError(QAbstractSocket::SocketError error);
    void onTcpDataReceived();
    void update();
    void connectionTimeout();
};

#endif // WAHOODIRCONTREADMILL_H
