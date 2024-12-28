#ifndef SPORTSPLUSROWER_H
#define SPORTSPLUSROWER_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QObject>
#include <QTime>

#include "devices/rower.h"
#include "virtualdevices/virtualbike.h"

class sportsplusrower : public rower {
    Q_OBJECT
  public:
    sportsplusrower(bool noWriteResistance, bool noHeartService);
    bool connected() override;

  private:
    double GetKcalFromPacket(const QByteArray &packet);
    double GetDistanceFromPacket(QByteArray packet);
    uint16_t GetElapsedFromPacket(const QByteArray &packet);
    void forceResistance(resistance_t requestResistance);
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                             bool wait_for_response);
    uint16_t wattsFromResistance(double resistance);
    void startDiscover();
    uint16_t watts() override;
    double GetWattFromPacket(const QByteArray &packet);

    QTimer *refresh;

    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    bool noWriteResistance = false;
    bool noHeartService = false;

    uint8_t firstVirtualBike = 0;
    bool firstCharChanged = true;
    QDateTime lastTimeCharChanged;
    uint8_t sec1update = 0;
    QByteArray lastPacket;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;
    QLowEnergyCharacteristic gattNotify2Characteristic;
    QLowEnergyCharacteristic gattNotify3Characteristic;

    bool initDone = false;
    bool initRequest = false;
    bool readyToStart = false;

    const resistance_t max_resistance = 24;

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

#endif // SPORTSPLUSROWER_H
