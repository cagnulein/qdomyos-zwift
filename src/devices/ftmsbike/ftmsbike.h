#ifndef FTMSBIKE_H
#define FTMSBIKE_H

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
#include "inclinationresistancetable.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

enum FtmsControlPointCommand {
    FTMS_REQUEST_CONTROL = 0x00,
    FTMS_RESET,
    FTMS_SET_TARGET_SPEED,
    FTMS_SET_TARGET_INCLINATION,
    FTMS_SET_TARGET_RESISTANCE_LEVEL,
    FTMS_SET_TARGET_POWER,
    FTMS_SET_TARGET_HEARTRATE,
    FTMS_START_RESUME,
    FTMS_STOP_PAUSE,
    FTMS_SET_TARGETED_EXP_ENERGY,
  FTMS_SET_TARGETED_STEPS,
    FTMS_SET_TARGETED_STRIDES,
    FTMS_SET_TARGETED_DISTANCE,
    FTMS_SET_TARGETED_TIME,
    FTMS_SET_TARGETED_TIME_TWO_HR_ZONES,
    FTMS_SET_TARGETED_TIME_THREE_HR_ZONES,
    FTMS_SET_TARGETED_TIME_FIVE_HR_ZONES,
    FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS,
    FTMS_SET_WHEEL_CIRCUMFERENCE,
    FTMS_SPIN_DOWN_CONTROL,
    FTMS_SET_TARGETED_CADENCE,
    FTMS_RESPONSE_CODE = 0x80
};

enum FtmsResultCode {
    FTMS_SUCCESS = 0x01,
    FTMS_NOT_SUPPORTED,
    FTMS_INVALID_PARAMETER,
    FTMS_OPERATION_FAILED,
    FTMS_CONTROL_NOT_PERMITTED
};

class ftmsbike : public bike {
    Q_OBJECT
  public:
    ftmsbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset, double bikeResistanceGain);
    ~ftmsbike();
    bool connected() override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    resistance_t maxResistance() override { return max_resistance; }
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    double maxGears() override;
    double minGears() override;

    // true because or the bike supports it by hardware or because QZ is emulating this in this module
    bool ergModeSupportedAvailableBySoftware() override { return true; }
    bool inclinationAvailableBySoftware() override { return !resistance_lvl_mode; }

  private:
    bool writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void writeCharacteristicZwiftPlay(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void zwiftPlayInit();
    void startDiscover();
    void setWheelDiameter(double diameter);
    uint16_t watts() override;
    void init();
    void forceResistance(resistance_t requestResistance);
    void forcePower(int16_t requestPower);
    void forceInclination(double requestInclination);
    void sendZwiftPlayInclination(double inclination);
    uint16_t wattsFromResistance(double resistance);

    QTimer *refresh;
    
    // Gear modification constants
    static constexpr int GEARS_SLOPE_MULTIPLIER = 50;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;

    QLowEnergyCharacteristic zwiftPlayWriteChar;
    QLowEnergyService *zwiftPlayService = nullptr;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QByteArray lastPacketFromFTMS;
    QDateTime lastRefreshCharacteristicChangedPower = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChanged2AD2 = QDateTime::currentDateTime();
    QDateTime lastRefreshCharacteristicChanged2ACE = QDateTime::currentDateTime();
    bool ftmsFrameReceived = false;
    uint8_t firstStateChanged = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;
    double lastGearValue = -1;
    int max_resistance = 100;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    bool powerForced = false;

    bool resistance_lvl_mode = false;
    bool resistance_received = false;
    inclinationResistanceTable _inclinationResistanceTable;

    // D500V2 workaround: track if we're awaiting start simulation command after request control
    bool awaiting_start_simulation_after_request_control = false;

    bool DU30_bike = false;
    bool ICSE = false;
    bool DOMYOS = false;
    bool D500V2 = false;
    bool _3G_Cardio_RB = false;
    bool SCH_190U = false;
    bool SCH_290R = false;
    bool D2RIDE = false;
    bool WATTBIKE = false;
    bool VFSPINBIKE = false;
    bool SS2K = false;
    bool DIRETO_XR = false;
    bool JFBK5_0 = false;
    bool BIKE_ = false;
    bool SMB1 = false;
    bool LYDSTO = false;
    bool DMASUN = false;
    bool SL010 = false;
    bool REEBOK = false;
    bool TITAN_7000 = false;
    bool T2 = false;
    bool FIT_BK = false;
    bool YS_G1MPLUS = false;
    bool EXPERT_SX9 = false;
    bool PM5 = false;
    bool THINK_X = false;
    bool WLT8828 = false;
    bool VANRYSEL_HT = false;
    bool MAGNUS = false;
    bool MRK_S26C = false;
    bool HAMMER = false;
    bool YPBM = false;
    bool SPORT01 = false;
    bool FS_YK = false;
    bool S18 = false;
    bool ZIPRO_RAVE = false;
    bool SPEEDRACEX = false;

    uint8_t secondsToResetTimer = 5;

    int16_t T2_lastGear = 0;

    uint8_t battery_level = 0;

    bool wattReceived = false;
    bool gearInclinationSent = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;
    QDateTime lastGoodCadence = QDateTime::currentDateTime();

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

#endif // FTMSBIKE_H
