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

#include "bike.h"
#include "virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class trxappgateusbbike : public bike {
    Q_OBJECT
  public:
    trxappgateusbbike(bool noWriteResistance, bool noHeartService, uint8_t bikeResistanceOffset,
                      double bikeResistanceGain);
    bool connected();

    void *VirtualBike();
    void *VirtualDevice();

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
    uint16_t watts();
    double GetWattFromPacket(const QByteArray &packet);
    double GetWattFromPacketFytter(const QByteArray &packet);
    double GetCadenceFromPacket(const QByteArray &packet);

    QTimer *refresh;
    virtualbike *virtualBike = nullptr;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

    uint8_t bikeResistanceOffset = 4;
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
    } TYPE;
    TYPE bike_type = TRXAPPGATE;

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

#endif // TRXAPPGATEUSBBIKE_H
