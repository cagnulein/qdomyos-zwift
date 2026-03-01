#ifndef WAHOODIRCONBIKE_H
#define WAHOODIRCONBIKE_H

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

#include "devices/bike.h"
#include "dirconpacket.h"
#include "dircondiscovery.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

/**
 * @brief Bike device that connects via Wahoo DirCon protocol over TCP
 *
 * This class implements a bike device that connects to Wahoo DirCon
 * servers (e.g., KICKR, other Wahoo trainers, or QZ in server mode)
 * over the network instead of Bluetooth.
 */
class wahoodirconbike : public bike {
    Q_OBJECT

public:
    /**
     * @brief Constructor for Wahoo DirCon Bike
     * @param deviceInfo Device information from mDNS discovery
     * @param noWriteResistance Disable resistance writes
     * @param noHeartService Disable heart rate service
     * @param bikeResistanceOffset Resistance offset for calibration
     * @param bikeResistanceGain Resistance gain for calibration
     */
    wahoodirconbike(const DirconDeviceInfo &deviceInfo, bool noWriteResistance, bool noHeartService,
                    int8_t bikeResistanceOffset, double bikeResistanceGain);
    ~wahoodirconbike();

    // Override from bluetoothdevice
    bool connected() override;
    BLUETOOTH_TYPE deviceType() override { return BIKE; }

    // Override from bike
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t maxResistance() override { return max_resistance; }
    resistance_t resistanceFromPowerRequest(uint16_t power) override;

    // ERG mode support
    bool ergModeSupportedAvailableBySoftware() override { return true; }

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
    int8_t bikeResistanceOffset;
    double bikeResistanceGain;

    // Resistance
    resistance_t max_resistance;
    resistance_t lastResistance;
    resistance_t lastRequestResistance;

    // FTMS characteristic UUIDs
    static const quint16 UUID_FTMS_SERVICE = 0x1826;
    static const quint16 UUID_INDOOR_BIKE_DATA = 0x2AD2;
    static const quint16 UUID_CYCLING_POWER_MEASUREMENT = 0x2A63;
    static const quint16 UUID_HEART_RATE = 0x2A37;
    static const quint16 UUID_CSC_MEASUREMENT = 0x2A5B;
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
    void forceResistance(resistance_t requestResistance);
    void forcePower(int16_t requestPower);
    uint16_t watts() override;
    uint16_t wattsFromResistance(double resistance);

    // FTMS protocol helpers
    void parseIndoorBikeData(const QByteArray &value);
    void parseCyclingPowerMeasurement(const QByteArray &value);
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
    lockscreen *h = 0;
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

#endif // WAHOODIRCONBIKE_H
