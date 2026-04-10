#ifndef KAYAKFIRSTROWER_H
#define KAYAKFIRSTROWER_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QDateTime>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptor.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qtimer.h>

#include "devices/rower.h"

class kayakfirstrower : public rower {
    Q_OBJECT
  public:
    kayakfirstrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                    double bikeResistanceGain);
    bool connected() override;

  private:
    bool writeCommand(const QString &command, const QString &info, bool wait_for_response = false);
    void btinit();
    void parseLine(const QByteArray &line);
    uint16_t watts() override;

    QTimer *refresh = nullptr;

    bool noWriteResistance = false;
    bool noHeartService = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    bool initDone = false;
    bool initRequest = false;
    uint8_t firstVirtualBike = 0;
    uint8_t sec1Update = 0;

    QByteArray streamBuffer;
    QDateTime lastDataUpdate = QDateTime::currentDateTime();

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // KAYAKFIRSTROWER_H
