#ifndef CYCLEOPSPHANTOMBIKE_H
#define CYCLEOPSPHANTOMBIKE_H

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

class cycleopsphantombike : public bike {
    Q_OBJECT
  public:
    cycleopsphantombike(bool noWriteResistance, bool noHeartService);
    void changePower(int32_t power) override;
    bool connected() override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;

  private:
    enum class ControlMode : uint8_t {
        Headless = 0x00,
        ManualPower = 0x01,
        ManualSlope = 0x02,
        PowerRange = 0x03,
        WarmUp = 0x04,
        RollDown = 0x05
    };

    enum class ControlStatus : uint8_t {
        SpeedOkay = 0x00,
        SpeedUp = 0x01,
        SpeedDown = 0x02,
        RollDownInitializing = 0x03,
        RollDownInProcess = 0x04,
        RollDownPassed = 0x05,
        RollDownFailed = 0x06
    };
    void setControlMode(ControlMode mode, int16_t parameter1 = 0, int16_t parameter2 = 0);

    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    void forceInclination(double inclination);
    uint16_t watts() override;
    double bikeResistanceToPeloton(double resistance);
    void setUserConfiguration(double wheelDiameter, double gearRatio);

    QTimer *refresh;

    const int max_resistance = 100;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyCharacteristic gattWriteCharCustomId;
    QLowEnergyService *gattPowerService = nullptr;
    QLowEnergyService *gattCustomService = nullptr;
    // QLowEnergyCharacteristic gattNotify1Characteristic;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged2A5B = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChanged2AD2 = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChangedPower = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;
    uint16_t CrankRevsRead = 0;

    double lastGearValue = -1;
    bool resistance_received = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

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

    void powerPacketReceived(const QByteArray &b);
};

#endif // CYCLEOPSPHANTOMBIKE_H
