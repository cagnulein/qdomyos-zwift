#ifndef KETTLERC12BIKE_H
#define KETTLERC12BIKE_H

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

class kettlerc12bike : public bike {
    Q_OBJECT
  public:
    kettlerc12bike(bool noWriteResistance, bool noHeartService, bool testResistance,
                   int8_t bikeResistanceOffset, double bikeResistanceGain);
    ~kettlerc12bike() override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t maxResistance() override { return max_resistance; }
    bool connected() override;

  private:
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void btinit();
    void forceResistance(resistance_t requestResistance);
    void setTargetPower(uint16_t watts);
    void sendPoll();
    void startDiscover();
    uint16_t watts() override;

    const resistance_t max_resistance = 250; // Max 250 watts for ergometer
    QTimer *refresh;

    // BLE Services and Characteristics
    QLowEnergyService *gattCommunicationChannelService = nullptr;

    QLowEnergyCharacteristic gattWriteCharacteristic;     // Custom service write (0x2a28)
    QLowEnergyCharacteristic gattNotify0x0034;            // Cadence notifications
    QLowEnergyCharacteristic gattNotify0x003e;            // Power notifications
    QLowEnergyCharacteristic gattNotify0x0048;            // Speed notifications

    bool initDone = false;
    bool initRequest = false;
    bool noWriteResistance = false;
    bool noHeartService = false;
    bool testResistance = false;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    uint8_t sec1Update = 0;
    uint8_t counterPoll = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    // UUIDs for Kettler C12 - Custom Service
    const QBluetoothUuid KETTLER_SERVICE_UUID = QBluetoothUuid(QStringLiteral("8ce5cc01-0a4d-11e9-ab14-d663bd873d93"));
    const QBluetoothUuid KETTLER_WRITE_CHAR_UUID = QBluetoothUuid((quint16)0x2a28); // Software Revision String (repurposed for control)

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
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // KETTLERC12BIKE_H
