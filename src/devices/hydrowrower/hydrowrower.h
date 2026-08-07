#ifndef HYDROWROWER_H
#define HYDROWROWER_H

#include "devices/rower.h"
#include <QMutex>
#include <QThread>
#include <QTimer>

class HydrowSerialThread : public QThread {
    Q_OBJECT
  public:
    HydrowSerialThread(QObject *parent = nullptr);
    ~HydrowSerialThread() override;
    void stop();

  signals:
    void metrics(double cadence, double distanceMeters, double watts, double speedKph, double strokes);
    void debug(QString message);

  protected:
    void run() override;

  private:
    bool openPort();
    void closePort();
    void writeCommand(const QByteArray &command);
    void processBytes(const QByteArray &bytes);
    void parseLine(const QByteArray &line);

    QString activeDevicePath;
    int port = -1;
    QMutex mutex;
    bool running = false;
    QByteArray receiveBuffer;
    double lastDistanceMeters = 0.0;
    double lastCadence = 0.0;
    double lastWatts = 0.0;
    double lastSpeedKph = 0.0;
    double strokeCount = 0.0;
    qint64 lastDistanceTimestamp = 0;
};

class hydrowrower : public rower {
    Q_OBJECT
  public:
    hydrowrower(bool noWriteResistance, bool noHeartService, bool noVirtualDevice = false);
    ~hydrowrower() override;
    bool connected() override;
    uint16_t watts() override;

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  signals:
    void disconnected();
    void debug(QString message);

  private slots:
    void update();
    void onMetrics(double cadence, double distanceMeters, double watts, double speedKph, double strokes);

  private:
    QTimer *refresh = nullptr;
    HydrowSerialThread *serial = nullptr;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noVirtualDevice = false;
    bool connectedState = false;
    double currentWatts = 0.0;
    double lastDistanceMeters = 0.0;
    double lastSpeedKph = 0.0;
    qint64 lastMetricsTimestamp = 0;
};

#endif // HYDROWROWER_H
