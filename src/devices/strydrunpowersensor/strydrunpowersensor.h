#ifndef STRYDRUNPOWERSENSOR_H
#define STRYDRUNPOWERSENSOR_H

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

#include "treadmill.h"


#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class strydrunpowersensor : public treadmill {
    Q_OBJECT
  public:
    strydrunpowersensor(bool noWriteResistance, bool noHeartService, bool noVirtualDevice);
    bool connected() override;

  private:
    void writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    uint16_t watts();

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    // QLowEnergyCharacteristic gattNotify1Characteristic;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCadenceChanged = QDateTime::currentDateTime();
    QDateTime lastRefreshPowerChanged = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noVirtualDevice = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;
    uint16_t LastCrankEventTime = 0;
    double CrankRevs = 0;

    bool powerReceived = false;

    bool FORERUNNER = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void onHeartRate(uint8_t heart);

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
#endif // STRYDRUNPOWERSENSOR_H
