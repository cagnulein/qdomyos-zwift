#ifndef KETTLERRACERSBIKE_H
#define KETTLERRACERSBIKE_H

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

#ifdef Q_OS_ANDROID
#include "kettlerhandshakereader.h"
#endif

class kettlerracersbike : public bike {
    Q_OBJECT
  public:
    kettlerracersbike(bool noWriteResistance, bool noHeartService);
    ~kettlerracersbike();
    void changePower(int32_t power) override;
    bool connected() override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;

  private:
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    void sendHandshake(const QByteArray &seed);
    void requestHandshakeSeed();
    void subscribeKettlerNotifications();
    void forceInclination(double inclination);
    uint16_t watts() override;
    double bikeResistanceToPeloton(double resistance);

    QTimer *refresh;

    const int max_resistance = 100;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyCharacteristic gattWriteCharKettlerId;
    QLowEnergyCharacteristic gattKeyReadCharKettlerId;   // 638a1104
    QLowEnergyCharacteristic gattKeyWriteCharKettlerId;  // 638a1105
    QLowEnergyService *gattPowerService = nullptr;
    QLowEnergyService *gattKettlerService = nullptr;
    QLowEnergyService *gattCSCService = nullptr;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged2A5B = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChangedKettler = QDateTime::currentDateTime();
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
    bool primedNotifyStart = false;
    bool handshakeRequested = false;
    bool handshakeDone = false;
    bool notificationsSubscribed = false;
    bool kettlerServiceReady = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

#ifdef Q_OS_ANDROID
    KettlerHandshakeReader* androidHandshakeReader = nullptr;
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
    void cscPacketReceived(const QByteArray &b);
    void kettlerPacketReceived(const QByteArray &b);

#ifdef Q_OS_ANDROID
    void onAndroidDeviceConnected();
    void onAndroidDeviceDisconnected();
    void onAndroidHandshakeSeedReceived(const QByteArray& seedData);
    void onAndroidHandshakeReadError(const QString& error);
    void onAndroidDataReceived(const QString& characteristicUuid, const QByteArray& data);
#endif
};

#endif // KETTLERRACERSBIKE_H
