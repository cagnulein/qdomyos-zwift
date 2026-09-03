#ifndef HORIZONTREADMILL_H
#define HORIZONTREADMILL_H

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
#include "devices/horizonprotocol/horizonprofile.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class horizontreadmill : public treadmill {
    Q_OBJECT
  public:
    horizontreadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    double minStepInclination() override;
    double minStepSpeed() override;

    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;

  private:
    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic, uint8_t *data,
                             uint8_t data_len, QString info, bool disable_log = false, bool wait_for_response = false);
    void waitForAPacket();
    void startDiscover();
    void btinit();

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyCharacteristic gattWriteCharControlPointIdYpooMiniPro;
    QLowEnergyService *gattFTMSService = nullptr;
    QLowEnergyCharacteristic gattWriteCharCustomService;
    QLowEnergyService *gattCustomService = nullptr;
    QLowEnergyCharacteristic gattWriteCharMerachUnlock;
    QLowEnergyService *gattMerachUnlockService = nullptr;
    volatile int notificationSubscribed = 0;

    static inline const QBluetoothUuid DomyosServiceId{QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455")};

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QByteArray lastPacketComplete;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    bool firstDistanceCalculated = false;
    uint8_t firstStateChanged = 0;
    double lastSpeed = 0.0;
    double lastInclination = 0;
    qint64 lastNonZeroSpeedTimestamp = 0;
    bool horizonPaused = false;
    double lastHorizonForceSpeed = 0;
    double minInclination = 0.0;
    double maxInclination = 15.0;

    bool initDone = false;
    bool initRequest = false;
    bool initPacketRecv = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    int32_t customRecv = 0;
    int32_t messageID = 0;

    bool mobvoi_treadmill = false;
    bool mobvoi_tmp_treadmill = false;
    bool kettler_treadmill = false;
    bool wellfit_treadmill = false;
    bool sole_tt8_treadmill = false;
    bool sole_s77_treadmill = false;
    bool anplus_treadmill = false;
    bool tunturi_t60_treadmill = false;
    bool trx3500_treadmill = false;
    bool sole_f85_treadmill = false;
    bool sole_f89_treadmill = false;
    bool adidas_treadmill = false;
    bool schwinn_810_treadmill = false;
    bool yesoul_treadmill = false;
    bool technogymrun = false;
    bool disableAutoPause = false;
    bool HORIZON_78AT_treadmill = false;
    bool ICONCEPT_FTMS_treadmill = false;
    bool iconcept_ftms_treadmill_inclination_table = false;
    bool DOMYOS = false;
    bool domyos_treadmill_ts100 = false;
    bool SW_TREADMILL = false;
    bool BOWFLEX_T9 = false;
    bool YPOO_MINI_PRO = false;
    bool MX_TM = false;
    bool FIT = false;
    bool FIT_TM = false;
    bool T3G_PRO = false;
    bool T3G_ELITE = false;
    bool JFTM_T202 = false;
    bool TP1 = false;
    bool T01 = false;
    bool TM4800 = false;
    bool TM4500 = false;
    bool TM6500 = false;
    bool FS_TREADMILL = false;
    bool WT_TREADMILL = false;
    bool THERUN_T15 = false;
    bool MERACH_TREADMILL = false;
    bool checkIfForceSpeedNeeding(double requestSpeed);
    float float_one_point_round(float value);

    // profiles
    // The profile-upload handshake is shared with the Horizon ellipticals; see horizonprofile.h.
    horizonprofile profile;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

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
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // HORIZONTREADMILL_H
