#ifndef TRXAPPGATEUSBELLIPTICAL_H
#define TRXAPPGATEUSBELLIPTICAL_H

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

#include "devices/elliptical.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class trxappgateusbelliptical : public elliptical {
    Q_OBJECT
  public:
    trxappgateusbelliptical(bool noWriteResistance = false, bool noHeartService = false, int8_t bikeResistanceOffset = 4,
                   double bikeResistanceGain = 1.0);
    bool connected() override;
    bool inclinationAvailableByHardware() override;

  private:
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    uint16_t watts() override;
    void forceResistance(resistance_t requestResistance);
    void btinit();
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetCadenceFromPacket(const QByteArray &packet);
    double GetWattFromPacket(const QByteArray &packet);

    QTimer *refresh;

    QLowEnergyService* gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastValidPacketTime = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    const uint8_t max_resistance = 72; // 24;
    const uint8_t default_resistance = 6;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    uint8_t counterPoll = 0;
    bool searchStopped = false;

    typedef enum TYPE {
        ELLIPTICAL_GENERIC = 0,
        DCT2000I = 1,
        JTX_FITNESS = 2,
        TAURUS_FX99 = 3,
    } TYPE;
    TYPE elliptical_type = ELLIPTICAL_GENERIC;


#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  Q_SIGNALS:
    void disconnected();
    void debug(QString string);

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
    void changeInclinationRequested(double grade, double percentage);
    // void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // TRXAPPGATEUSBELLIPTICAL_H
