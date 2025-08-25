#ifndef SOLEBIKE_H
#define SOLEBIKE_H

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

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class solebike : public bike {
    Q_OBJECT
  public:
    solebike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

  private:
    bool r92 = false;
    const resistance_t max_resistance = 40;
    double bikeResistanceToPeloton(double resistance);
    double GetDistanceFromPacket(const QByteArray &packet);
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetWattFromPacket(const QByteArray &packet);
    QTime GetElapsedFromPacket(const QByteArray &packet);
    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    void forceResistance(resistance_t requestResistance);
    void sendPoll();
    uint16_t watts() override;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    uint8_t counterPoll = 1;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastMetricUpdate = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    resistance_t lastResistanceBeforeDisconnection = -1;
    bool communicationTimeout = false;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  Q_SIGNALS:
    void disconnected();

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

#endif // SOLEBIKE_H
