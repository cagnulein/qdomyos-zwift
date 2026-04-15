#ifndef LIFESPANBIKE_H
#define LIFESPANBIKE_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QDateTime>
#include <QObject>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include "bike.h"

class lifespanbike : public bike {
    Q_OBJECT

  public:
    lifespanbike(bool noWriteResistance = false, bool noHeartService = false, int8_t bikeResistanceOffset = 4,
                 double bikeResistanceGain = 1.0);
    bool connected() override;
    uint16_t watts() override;

  private:
    enum class CommandState {
        None,
        QueryStatus,
        QueryUnknown81,
        QueryCadence,
        QueryUnknown86,
        QueryCalories,
        QueryDistance,
        QueryTime,
        QuerySpeed,
        QueryPower
    };

    uint16_t data16(const QByteArray &packet) const;
    uint16_t adjustedLifespanWatts();
    double GetSpeedFromPacket(const QByteArray &packet) const;
    void btinit();
    void forceResistance(resistance_t requestResistance);
    void sendPoll();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);

    bool noWriteResistance = false;
    bool noHeartService = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    uint8_t firstInit = 0;
    uint8_t pollIndex = 0;
    QByteArray lastPacket;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;

    QTimer *refresh = nullptr;
    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;
    bool initDone = false;
    bool initRequest = false;
    CommandState currentCommand = CommandState::None;

  Q_SIGNALS:
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

#endif // LIFESPANBIKE_H
