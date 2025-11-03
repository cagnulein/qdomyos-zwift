#ifndef YPOOELLIPTICAL_H
#define YPOOELLIPTICAL_H

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
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class ypooelliptical : public elliptical {
    Q_OBJECT
  public:
    ypooelliptical(bool noWriteResistance = false, bool noHeartService = false, int8_t bikeResistanceOffset = 4,
                   double bikeResistanceGain = 1.0);
    bool connected() override;
    double minStepInclination() override;
    bool inclinationSeparatedFromResistance() override;

  private:
    void writeCharacteristic(QLowEnergyCharacteristic* characteristic, QLowEnergyService *service, uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    uint16_t watts();
    void forceResistance(resistance_t requestResistance);
    void forceInclination(double inclination);

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattCustomService = nullptr;

    QLowEnergyCharacteristic gattFTMSWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChanged2AD2 = QDateTime::currentDateTime();
    QDateTime lastStrideCountChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t lastStrideCount = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    const uint8_t max_resistance = 72; // 24;
    const uint8_t default_resistance = 6;
    metric instantCadence;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    uint8_t counterPoll = 0;
    bool SCH_590E = false;
    bool SCH_411_510E = false;
    bool E35 = false;
    bool KETTLER = false;
    bool CARDIOPOWER_EEGO = false;
    bool MYELLIPTICAL = false;
    bool SKANDIKA = false;
    bool DOMYOS = false;
    bool FEIER = false;
    bool MX_AS = false;
    bool FTMS = false;

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
    void changeInclinationRequested(double grade, double percentage);
    // void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // YPOOELLIPTICAL_H
