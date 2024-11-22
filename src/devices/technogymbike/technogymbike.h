#ifndef TECHNOGYMBIKE_H
#define TECHNOGYMBIKE_H

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

#include "wheelcircumference.h"
#include "devices/bike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class technogymbike : public bike {
    Q_OBJECT
  public:
    technogymbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain);
    bool connected() override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t maxResistance() override { return max_resistance; }
    resistance_t resistanceFromPowerRequest(uint16_t power) override;

  private:
    bool writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    uint16_t watts() override;
    void init();
    void forceResistance(resistance_t requestResistance);
    void forcePower(int16_t requestPower);
    uint16_t wattsFromResistance(double resistance);

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;

    QLowEnergyCharacteristic customWriteChar;
    QLowEnergyService *customService = nullptr;
    QBluetoothUuid charWrite        = QBluetoothUuid(QStringLiteral("58094966-498C-470D-8051-37E617A13895"));
    QBluetoothUuid serviceUuid      = QBluetoothUuid(QStringLiteral("a913bfc0-929e-11e5-b928-0002a5d5c51b"));

    bool powerForced = false;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QByteArray lastPacketFromFTMS;
    QDateTime lastCadenceChanged = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint16_t lastCadenceValue = 0;
    uint8_t firstStateChanged = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    double lastGearValue = -1;
    int max_resistance = 100;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    bool noVirtualDevice = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;


#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  Q_SIGNALS:
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

#endif // TECHNOGYMBIKE_H
