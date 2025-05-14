#ifndef TRXAPPGATEUSBBIKE_H
#define TRXAPPGATEUSBBIKE_H

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

#include "devices/bike.h"
#include "ergtable.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class trxappgateusbbike : public bike {
    Q_OBJECT
  public:
    trxappgateusbbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                      double bikeResistanceGain);
    bool connected() override;
    resistance_t maxResistance() override { return 32; }
    resistance_t resistanceFromPowerRequest(uint16_t power) override;

  private:
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetResistanceFromPacket(const QByteArray &packet);
    double GetKcalFromPacket(const QByteArray &packet);
    double GetDistanceFromPacket(QByteArray packet);
    uint16_t GetElapsedFromPacket(const QByteArray &packet);
    void forceResistance(resistance_t requestResistance);
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                             bool wait_for_response);
    void startDiscover();
    uint16_t watts() override;
    double GetWattFromPacket(const QByteArray &packet);
    double GetWattFromPacketFytter(const QByteArray &packet);
    double GetCadenceFromPacket(const QByteArray &packet);
    uint16_t wattsFromResistance(double resistance);

    QTimer *refresh;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    bool noWriteResistance = false;
    bool noHeartService = false;

    uint8_t firstVirtualBike = 0;
    bool firstCharChanged = true;
    QTime lastTimeCharChanged;
    uint8_t sec1update = 0;
    QByteArray lastPacket;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;
    QLowEnergyCharacteristic gattNotify2Characteristic;

    bool initDone = false;
    bool initRequest = false;
    bool readyToStart = false;

    typedef enum TYPE {
        TRXAPPGATE = 0,
        IRUNNING = 1,
        CHANGYOW = 2,
        SKANDIKAWIRY = 3,
        ICONSOLE = 4,
        JLL_IC400 = 5,
        DKN_MOTION = 6,
        DKN_MOTION_2 = 7,
        ASVIVA = 8,
        FYTTER_RI08 = 9,
        ICONSOLE_2 = 10,
        HERTZ_XR_770 = 11,
        CASALL = 12,
        VIRTUFIT = 13,
        HERTZ_XR_770_2 = 14,
        VIRTUFIT_2 = 15,
        TUNTURI = 16,
        TUNTURI_2 = 17,
        FITHIWAY = 18,
        ENERFIT_SPX_9500 = 19,
        HOP_SPORT_HS_090H = 20,
        ENERFIT_SPX_9500_2 = 21,
        REEBOK = 22,
        REEBOK_2 = 23,
        BIKZU = 24,
        TOORX_SRX_500 = 25,
        IRUNNING_2 = 26,
        PASYOU = 27,
        FAL_SPORTS = 28,
        HAMMER_SPEED_BIKE_S = 29,
    } TYPE;
    TYPE bike_type = TRXAPPGATE;

    // SmartSpin2k
    metric ResistanceFromFTMSAccessory;
    uint64_t ResistanceFromFTMSAccessoryLastTime = 0;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void resistanceFromFTMSAccessory(resistance_t res) override;

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

#endif // TRXAPPGATEUSBBIKE_H
