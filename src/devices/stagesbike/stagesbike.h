#ifndef STAGES_BIKE_H
#define STAGES_BIKE_H

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

class stagesbike : public bike {
    Q_OBJECT
  public:
    stagesbike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    bool connected() override;
    resistance_t maxResistance() override { return 100; }
    bool ergManagedBySS2K() override { return true; }

  private:
    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                           uint8_t *data, uint8_t data_len, QString info, bool disable_log = false,
                                           bool wait_for_response = false);
    uint16_t wattsFromResistance(double resistance);
    metric ResistanceFromFTMSAccessory;
    uint64_t ResistanceFromFTMSAccessoryLastTime = 0;
    void startDiscover();
    uint16_t watts() override;

    // Elite Methods
    QByteArray setTargetPower(quint16 watts);
    QByteArray setBrakeLevel(double level);
    QByteArray setSimulationMode(double grade, double crr, double wrc, double windSpeedKPH, double draftingFactor);

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;

    QLowEnergyService* eliteService = nullptr;
    QLowEnergyCharacteristic eliteWriteCharacteristic;
    QLowEnergyCharacteristic eliteWriteCharacteristic2;

    double lastGearValue = 0;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noVirtualDevice = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void resistanceFromFTMSAccessory(resistance_t res) override;

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void inclinationChanged(double grade, double percentage);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // STAGES_BIKE_H
