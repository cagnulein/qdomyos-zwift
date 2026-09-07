#ifndef XCXBIKE_H
#define XCXBIKE_H

#include "devices/bike.h"

#include <QBluetoothDeviceInfo>
#include <QByteArray>
#include <QDateTime>
#include <QLowEnergyCharacteristic>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QTimer>

struct XcxTelemetry {
    XcxTelemetry(quint8 state1 = 0, quint8 state2 = 0, quint16 timer = 0, double speedKph = 0, quint32 distanceRaw = 0,
                 quint16 energy = 0, quint16 cadence = 0, quint16 unknown15 = 0, quint16 power = 0,
                 quint16 resistance = 0)
        : state1(state1), state2(state2), timer(timer), speedKph(speedKph), distanceRaw(distanceRaw), energy(energy),
          cadence(cadence), unknown15(unknown15), power(power), resistance(resistance) {}

    quint8 state1 = 0;
    quint8 state2 = 0;
    quint16 timer = 0;
    double speedKph = 0;
    quint32 distanceRaw = 0;
    quint16 energy = 0;
    quint16 cadence = 0;
    quint16 unknown15 = 0;
    quint16 power = 0;
    quint16 resistance = 0;
};

class xcxbike : public bike {
    Q_OBJECT

  public:
    xcxbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain);

    static bool parseTelemetry(const QByteArray &packet, XcxTelemetry *telemetry);
    bool connected() override;

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  signals:
    void disconnected();

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &value);
    void serviceScanDone();
    void serviceStateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void update();

  private:
    void createVirtualBike();
    uint16_t watts() override;

    QTimer refresh;
    QLowEnergyService *fff0Service = nullptr;
    QLowEnergyCharacteristic fff5WriteCharacteristic;
    QLowEnergyCharacteristic fff6NotifyCharacteristic;
    bool noWriteResistance;
    bool noHeartService;
    int8_t bikeResistanceOffset;
    double bikeResistanceGain;
    bool notificationEnabled = false;
    quint8 lastState1 = 0;
    quint8 lastState2 = 0;
    bool receivedState = false;
    QDateTime lastTelemetryTime;
    double partialCrankRevolution = 0;
};

#endif // XCXBIKE_H
