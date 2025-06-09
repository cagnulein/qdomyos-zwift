#ifndef DOMYOSBIKE_H
#define DOMYOSBIKE_H

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

#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class domyosbike : public bike {
    Q_OBJECT
  public:
    domyosbike(bool noWriteResistance = false, bool noHeartService = false, bool testResistance = false,
               int8_t bikeResistanceOffset = 4, double bikeResistanceGain = 1.0);
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t maxResistance() override { return max_resistance; }
    ~domyosbike() override;
    bool connected() override;

  private:
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetInclinationFromPacket(QByteArray packet);
    double GetKcalFromPacket(const QByteArray &packet);
    double GetDistanceFromPacket(const QByteArray &packet);
    uint16_t wattsFromResistance(double resistance);
    void forceResistance(resistance_t requestResistance);
    void updateDisplay(uint16_t elapsed);
    void btinit_changyow(bool startTape);
    void btinit_telink(bool startTape);
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    uint16_t watts() override;

    const resistance_t max_resistance = 15;
    QTimer *refresh;
    uint8_t firstVirtual = 0;
    uint8_t firstStateChanged = 0;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    volatile bool incompletePackets = false;
    bool initDone = false;
    bool initRequest = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool testResistance = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bool searchStopped = false;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    bool firstCharacteristicChanged = true;

    enum _BIKE_TYPE {
        CHANG_YOW,
        TELINK,
    };
    _BIKE_TYPE bike_type = CHANG_YOW;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void searchingStop();

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

#endif // DOMYOSBIKE_H
