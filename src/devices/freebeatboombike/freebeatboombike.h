#ifndef FREEBEATBOOMBIKE_H
#define FREEBEATBOOMBIKE_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QDateTime>
#include <QObject>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qtimer.h>

#include "bike.h"

class freebeatboombike : public bike {
    Q_OBJECT

  public:
    struct Telemetry {
        int rpm = 0;
        int resistance = 0;
        double speed = 0.0;
    };

    freebeatboombike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                     double bikeResistanceGain);

    bool connected() override;
    uint16_t watts() override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;

    static bool parseTelemetry(const QByteArray &packet, Telemetry *telemetry);
    static QByteArray queryCommand();
    static QByteArray streamCommand(bool enabled);
    static QByteArray resistanceCommand(int resistance);

  private:
    static constexpr double wheelDiameter = 0.129;
    static constexpr double wheelMultiplier = 7.39;
    static constexpr double powerMultiplier = 4.56;
    static constexpr resistance_t maxResistance = 100;

    void btinit();
    void writeCharacteristic(const QByteArray &data, const QString &info);
    void forceResistance(resistance_t requestResistance);
    void innerWriteResistance();
    void updateTelemetry(const Telemetry &telemetry);

    bool noWriteResistance = false;
    bool noHeartService = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bool initDone = false;
    bool initRequest = false;
    bool firstCharacteristicChanged = true;
    QDateTime lastRefreshCharacteristicChanged;

    QTimer *refresh = nullptr;
    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

  Q_SIGNALS:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone();
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError err);
};

#endif // FREEBEATBOOMBIKE_H
