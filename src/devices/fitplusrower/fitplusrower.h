#ifndef FITPLUSROWER_H
#define FITPLUSROWER_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qtimer.h>

#include "devices/rower.h"

class fitplusrower : public rower {
    Q_OBJECT

  public:
    fitplusrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                 double bikeResistanceGain);
    resistance_t maxResistance() override { return max_resistance; }
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    bool connected() override;

  private:
    resistance_t max_resistance = 24;

    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void forceResistance(resistance_t requestResistance);
    double rowerResistanceToPeloton(double resistance);
    uint16_t watts() override;

    QTimer *refresh = nullptr;
    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    uint8_t counterPoll = 1;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    resistance_t lastResistanceBeforeDisconnection = -1;
    bool requestResistanceCompleted = true;

    bool initDone = false;
    bool initRequest = false;
    bool noWriteResistance = false;
    bool noHeartService = false;

  Q_SIGNALS:
    void disconnected();

  public Q_SLOTS:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private Q_SLOTS:
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

#endif // FITPLUSROWER_H
