#ifndef KETTLERCOACHROWER_H
#define KETTLERCOACHROWER_H

#include <QBluetoothDeviceInfo>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServiceInfo>
#include <QBluetoothSocket>
#include <QDateTime>
#include <QElapsedTimer>
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
    resistance_t maxResistance() override;
    void clearStats() override;

  public slots:
    void changeResistance(resistance_t resistance) override;

    void deviceDiscovered(const QBluetoothDeviceInfo &device);
#ifdef Q_OS_ANDROID
    // Known paired MAC: connect over SPP without requiring discoverability.
    void connectToBondedDevice(const QBluetoothDeviceInfo &device);
#endif

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
        bool expectsResponse = true;
    };

    void createSocket();
    void enqueueRead(quint16 function);
    void enqueueWrite(quint16 function, const QByteArray &payload);
    void enqueueResistanceRequest(resistance_t requested);
    void sendNextRequest();
    void processFrame(const kettlerclassicprotocol::Frame &frame);
    void processTelemetry(const QByteArray &payload);
    void resetStrokeMetrics();
    void updateStrokeIdle(qint64 nowMs);
    void recordReportedResistance(quint8 level, const char *source);
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
    bool telemetrySeen = false;

    QBluetoothDeviceInfo bluetoothDevice;
    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;
    QBluetoothSocket *socket = nullptr;
    QByteArray socketReadBuffer;
    QQueue<Request> requestQueue;
    Request activeRequest;
    QTimer *refresh = nullptr;
    QTimer requestTimeoutTimer;

    quint16 currentWatts = 0;
    QElapsedTimer powerClock;
    kettlerclassicprotocol::CoachSStrokePowerSmoother strokePower;
    kettlerclassicprotocol::CoachSSessionCounters sessionCounters;
    kettlerclassicprotocol::CoachSPowerBasedPace powerBasedPace;
    bool lastPowerBasedMode = false;
    double rawDistanceOffsetMeters = 0.0;
    int pendingResistanceLevel = -1;
    int lastReportedResistanceLevel = -1;
    quint16 lastStrokes = 0;
    // Raw distance at the last completed stroke, independent of the
    // QZ workout's session counters. The elapsed clock is monotonic.
    quint16 lastStrokeDistanceMeters = 0;
    qint64 lastStrokeMs = -1;
};

#endif // KETTLERCOACHROWER_H
