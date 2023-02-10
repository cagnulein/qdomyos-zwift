#ifndef renphobike_H
#define renphobike_H

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

#include <QDateTime>
#include <QObject>
#include <QString>

#include "bike.h"
#include "ftmsbike.h"
#include "virtualbike.h"

class renphobike : public bike {
    Q_OBJECT
  public:
    renphobike(bool noWriteResistance, bool noHeartService);
    resistance_t pelotonToBikeResistance(int pelotonResistance);
    // uint8_t resistanceFromPowerRequest(uint16_t power);
    bool connected();
    resistance_t maxResistance() { return max_resistance; }

    void *VirtualBike();
    void *VirtualDevice();

  private:
    const resistance_t max_resistance = 80;
    double bikeResistanceToPeloton(double resistance);
    void writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    uint16_t ergModificator(uint16_t powerRequested);
    uint16_t watts();
    void forceResistance(resistance_t requestResistance);
    void forcePower(int16_t requestPower);

    QTimer *refresh;
    virtualbike *virtualBike = 0;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    
    QByteArray lastFTMSPacketReceived;
    resistance_t lastRequestResistance = -1;
    double lastPowerRequestedFactor = 1;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    metric wattFromBike;

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);

    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // renphobike_H
