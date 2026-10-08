#ifndef LIFEFITNESSTREADMILL_H
#define LIFEFITNESSTREADMILL_H

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
//#include <QtBluetooth/private/qlowenergycontrollerbase_p.h>
//#include <QtBluetooth/private/qlowenergyserviceprivate_p.h>
#include <QBluetoothDeviceDiscoveryAgent>
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

#include "treadmill.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class lifefitnesstreadmill : public treadmill {
    Q_OBJECT
  public:
    struct FtmsTreadmillData {
        bool valid = false;
        bool hasSpeed = false;
        double speedKmh = 0.0;
        bool hasAverageSpeed = false;
        double averageSpeedKmh = 0.0;
        bool hasDistance = false;
        double distanceMeters = 0.0;
        bool hasInclination = false;
        double inclinationPercent = 0.0;
        bool hasEnergy = false;
        double totalEnergyKcal = 0.0;
        bool hasHeartRate = false;
        double heartRateBpm = 0.0;
        bool hasElapsedTime = false;
        int elapsedSeconds = 0;
    };

    lifefitnesstreadmill(bool noWriteResistance, bool noHeartService);
    static FtmsTreadmillData parseFtmsTreadmillData(const QByteArray &data);
    bool connected() override;
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    double minStepInclination() override;
    double minStepSpeed() override;
    virtual bool canStartStop()  override{ return false; }
    bool canHandleSpeedChange() override { return false; }
    bool canHandleInclineChange() override { return false; }    

  private:
    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic, uint8_t *data,
                             uint8_t data_len, QString info, bool disable_log = false, bool wait_for_response = false);
    void waitForAPacket();
    bool waitForLifeFitnessState(const QByteArray &expectedState, int timeoutMs = 10000);
    void startDiscover();
    void btinit();

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;
    QLowEnergyCharacteristic gattWriteChar1CustomService1;
    QLowEnergyCharacteristic gattWriteChar2CustomService1;
    QLowEnergyCharacteristic gattWriteChar3CustomService2;
    QLowEnergyCharacteristic gattWriteChar4CustomService2;
    QLowEnergyCharacteristic gattCurrentStateCharacteristic;
    QLowEnergyService *gattCustomService1 = nullptr;
    QLowEnergyService *gattCustomService2 = nullptr;
    volatile int notificationSubscribed = 0;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QByteArray lastPacketComplete;
    QByteArray lastLifeFitnessState;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    bool firstDistanceCalculated = false;
    uint8_t firstStateChanged = 0;
    double lastSpeed = 0.0;
    double lastInclination = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    bool lifet5 = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
    void lifeFitnessStateChanged(const QByteArray &state);

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

    void changeInclinationRequested(double grade, double percentage);
};

#endif // LIFEFITNESSTREADMILL_H
