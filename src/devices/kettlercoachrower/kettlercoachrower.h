#ifndef KETTLERCOACHROWER_H
#define KETTLERCOACHROWER_H

#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>
#include <QDateTime>
#include <QQueue>
#include <QTimer>

#include "devices/rower.h"
#include "devices/kettlerclassicbike/kettlerclassicprotocol.h"

// Kettler Coach S (BR/EDR, SPP). It uses the Classic frame/CRC format,
// but the Coach S command set is distinct from that of Kettler bikes.
class kettlercoachrower : public rower {
    Q_OBJECT

  public:
    kettlercoachrower(bool noWriteResistance, bool noHeartService);
    ~kettlercoachrower() override;
    bool connected() override;
    uint16_t watts() override;

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  signals:
    void debug(QString message);
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
        quint8 operation = 0x01;
        QByteArray payload;
    };

    void createSocket();
    void enqueueRead(quint16 function);
    void sendNextRequest();
    void processFrame(const kettlerclassicprotocol::Frame &frame);
    void processTelemetry(const QByteArray &payload);
    void disconnectAfterError(const QString &reason);
    static quint16 be16(const QByteArray &data, int offset);

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool initDone = false;
    bool requestInFlight = false;
    bool awaitingHandshake = false;
    bool signalledConnection = false;
    bool signalledDisconnection = false;
    bool virtualCreated = false;

    QBluetoothDeviceInfo bluetoothDevice;
    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;
    QByteArray socketReadBuffer;
    QQueue<Request> requestQueue;
    Request activeRequest;
    QTimer *refresh = nullptr;
    QTimer requestTimeoutTimer;

    quint16 currentWatts = 0;
    quint16 lastStrokes = 0;
    quint16 lastDistanceMeters = 0;
    QDateTime lastStrokeTime;
    QDateTime lastDistanceTime;
};

#endif // KETTLERCOACHROWER_H
