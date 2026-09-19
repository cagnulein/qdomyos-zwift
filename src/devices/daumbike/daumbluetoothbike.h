#ifndef DAUMBLUETOOTHBIKE_H
#define DAUMBLUETOOTHBIKE_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>
#include <QDateTime>
#include <QSettings>
#include <QTimer>

#include "devices/bike.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualbike.h"

class daumbluetoothbike : public bike {
    Q_OBJECT
  public:
    explicit daumbluetoothbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                               double bikeResistanceGain);
    ~daumbluetoothbike() override;

    static bool matchesBluetoothName(const QString &name);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

    void changeInclination(double grade, double percentage) override;
    void forceInclination(double inclination);

  protected:
    bool supportsNativeInclination() const override { return false; }
    bool applyGearModifier() const override { return false; }

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void serviceCanceled();
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void serviceFinished();
    void readSocket();
    void rfCommConnected();
    void onSocketErrorOccurred(QBluetoothSocket::SocketError error);
    void update();

  private:
    static constexpr const char *serialPortUuid = "00001101-0000-1000-8000-00805f9b34fb";
    static constexpr int telemetryLength = 19;

    void sendBytes(const QByteArray &bytes);
    void sendInitPhase();
    void processInput();
    void processInitializationResponse();
    void processTelemetry();
    void pollTelemetry();
    void writePowerTarget(double power);
    void forceResistance(double requestResistance);
    void innerWriteResistance();
    void setTelemetry(const QByteArray &packet);
    static int powerStep(double power);
    uint16_t watts() override;

    resistance_t max_resistance = 800;
    resistance_t min_resistance = 25;
    QBluetoothDeviceInfo bluetoothDevice;
    QBluetoothServiceDiscoveryAgent *discoveryAgent = nullptr;
    QBluetoothServiceInfo serialPortService;
    QBluetoothSocket *socket = nullptr;
    QTimer *refresh = nullptr;
    QTimer *initTimer = nullptr;
    QByteArray socketReadBuffer;
    bool found = false;
    bool initDone = false;
    int initPhase = -1;
    uint8_t cockpitAddress = 0;
    bool writePower = false;
    double targetPower = 100.0;
    int lastPowerStep = -1;
    bool pollOutstanding = false;
    qint64 lastPollMs = 0;

    volatile double devicePower = 0.0;
    volatile double deviceHeartRate = 0.0;
    volatile double deviceCadence = 0.0;
    volatile double deviceSpeed = 0.0;
    volatile double deviceDistance = 0.0;
    volatile uint8_t deviceGear = 1;

    virtualbike *virtualBike = nullptr;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    bool noWriteResistance = false;
    bool noHeartService = false;
    FtmsControlPointCommand m_lastFtmsCommand = FTMS_REQUEST_CONTROL;

  signals:
    void disconnected();
    void debug(QString string);

  private slots:
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // DAUMBLUETOOTHBIKE_H
