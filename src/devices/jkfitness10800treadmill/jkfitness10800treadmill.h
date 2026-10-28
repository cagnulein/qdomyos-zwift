#ifndef JKFITNESS10800TREADMILL_H
#define JKFITNESS10800TREADMILL_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QByteArray>
#include <QDateTime>
#include <QQueue>
#include <QTimer>

#include "devices/treadmill.h"

class jkfitness10800treadmill : public treadmill {
    Q_OBJECT

  public:
    explicit jkfitness10800treadmill(bool noConsole = false, bool noHeartService = false);
    ~jkfitness10800treadmill() override;

    bool connected() override;
    bool canHandleSpeedChange() override { return true; }
    bool canHandleInclineChange() override { return true; }
    double minStepInclination() override;
    double minStepSpeed() override;
    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  signals:
    void disconnected();
    void debug(QString string);

  private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void serviceFinished();
    void readSocket();
    void rfCommConnected();
    void socketDisconnected();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError error);
    void setupTimeout();
    void update();
    void changeInclinationRequested(double grade, double percentage);

  private:
    void createSocket();
    void sendNextSetupFrame();
    void finishSetup();
    void writeFrame(const QByteArray &frame, const QString &info);
    void processFrame(const QByteArray &frame);
    void resetConnectionState();

    bool noConsole = false;
    bool noHeartService = false;
    bool initDone = false;
    bool setupWaiting = false;
    bool setupHadValidResponse = false;
    bool connectedSignalEmitted = false;
    bool disconnectedSignalEmitted = false;
    bool runningStateKnown = false;
    bool runningState = false;
    uint8_t pollCounter = 0;

    QBluetoothDeviceInfo bluetoothDevice;
    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;
    QByteArray socketReadBuffer;
    QQueue<QByteArray> setupQueue;

    QTimer *refresh = nullptr;
    QTimer setupTimeoutTimer;
    QDateTime lastTelemetryUpdate = QDateTime::currentDateTime();
};

#endif // JKFITNESS10800TREADMILL_H
