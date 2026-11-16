#ifndef KETTLERCLASSICBIKE_H
#define KETTLERCLASSICBIKE_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QByteArray>
#include <QDateTime>
#include <QQueue>
#include <QTimer>

#include "devices/bike.h"
#include "kettlerclassicprotocol.h"

class kettlerclassicbike : public bike {
    Q_OBJECT

  public:
    kettlerclassicbike(bool noWriteResistance, bool noHeartService, bool testResistance,
                       int8_t bikeResistanceOffset, double bikeResistanceGain);
    ~kettlerclassicbike() override;

    bool connected() override;
    resistance_t maxResistance() override;
    uint16_t watts() override;
    void setPaused(bool p) override;

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  signals:
    void debug(QString string);
    void disconnected();

  private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void serviceFinished();
    void rfCommConnected();
    void socketDisconnected();
    void readSocket();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError error);
    void requestTimeout();
    void update();

  private:
    struct Request {
        quint16 function = 0;
        quint8 subFunction = 0;
        QByteArray payload;
        bool expectsResponse = true;
    };

    static constexpr quint16 HANDSHAKE = 0x01;
    static constexpr quint16 FIRMWARE_VERSION = 0x02;
    static constexpr quint16 DEVICE_STATE = 0x06;
    static constexpr quint16 DEVICE_BRAKE_MODE = 0x07;
    static constexpr quint16 PULSE = 0x08;
    static constexpr quint16 RPM = 0x09;
    static constexpr quint16 POWER_TARGET = 0x0a;
    static constexpr quint16 POWER_CURRENT = 0x0b;
    static constexpr quint16 POWER_MIN = 0x0c;
    static constexpr quint16 POWER_MAX = 0x0d;
    static constexpr quint16 DISTANCE = 0x0f;
    static constexpr quint16 ENERGY = 0x10;
    static constexpr quint16 DEVICE_NAME = 0x14;
    static constexpr quint16 SERIAL_NUMBER = 0x15;
    static constexpr quint16 ARTICLE_NUMBER = 0x16;
    static constexpr quint16 CALIBRATION_VERSION = 0x17;
    static constexpr quint16 DEVICE_TYPE = 0x18;
    static constexpr quint16 DEVICE_IDENTIFIER = 0x19;
    static constexpr quint16 BRAKE_LEVEL = 0x1b;
    static constexpr quint16 BRAKE_LEVEL_MIN = 0x1c;
    static constexpr quint16 BRAKE_LEVEL_MAX = 0x1d;
    static constexpr quint16 START_STOP = 0x26;

    static constexpr quint8 SUB_READ = 0x01;
    static constexpr quint8 SUB_WRITE = 0x02;
    static constexpr quint8 SUB_ANSWER = 0x03;
    static constexpr quint8 SUB_STATUS = 0x04;
    static constexpr quint8 SUB_ERROR = 0x05;
    static constexpr quint8 SUB_RESET = 0x06;

    void createSocket();
    void enqueueRequest(quint16 function, quint8 subFunction, const QByteArray &payload = QByteArray());
    void enqueueRead(quint16 function);
    void sendNextRequest();
    void abortSetup(const QString &reason);
    void processFrame(const kettlerclassicprotocol::Frame &frame);
    void handleAnswer(const kettlerclassicprotocol::Frame &frame);
    void finishSetup();
    void enqueueSetupRequests();
    void enqueueTelemetryRequests();
    void enqueueResistanceRequest(resistance_t resistance);
    void enqueueWorkoutState(bool paused);
    void updateMetricsFromCadence();
    static quint32 readBigEndian(const QByteArray &payload);
    static QByteArray uint16Bytes(quint16 value);

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool testResistance = false;
    int8_t bikeResistanceOffset = 0;
    double bikeResistanceGain = 1.0;

    QBluetoothDeviceInfo bluetoothDevice;
    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;
    QByteArray socketReadBuffer;

    QQueue<Request> requestQueue;
    Request activeRequest;
    bool requestInFlight = false;
    bool initDone = false;
    bool setupQueued = false;
    bool connectedSignalEmitted = false;
    bool disconnectedSignalEmitted = false;

    QTimer *refresh = nullptr;
    QTimer requestTimeoutTimer;
    QDateTime lastTelemetryUpdate = QDateTime::currentDateTime();

    quint16 rpm = 0;
    quint16 power = 0;
    quint16 pulse = 0;
    quint16 minPower = 25;
    quint16 maxPower = 600;
    quint8 minLevel = 1;
    quint8 maxLevel = 32;
    quint8 level = 1;
    int diameterInMM = 2000;
    bool brakeModeLevel = true;
    bool firstStateChanged = false;
};

#endif // KETTLERCLASSICBIKE_H
