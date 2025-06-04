#ifndef FITSHOWTREADMILL_H
#define FITSHOWTREADMILL_H

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
#include <QtCore/qstring.h>

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

#include "treadmill.h"
#include "virtualdevices/virtualtreadmill.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

#define FITSHOW_PKT_HEADER 2
#define FITSHOW_PKT_FOOTER 3

#define FITSHOW_SYS_CONTROL 83
#define FITSHOW_SYS_INFO 0x50
#define FITSHOW_SYS_DATA 0x52
#define FITSHOW_SYS_STATUS 81
#define FITSHOW_SYS_KEY 84
#define FITSHOW_CONTROL_TARGET_OR_RUN 2
#define FITSHOW_CONTROL_READY_OR_START 1
#define FITSHOW_CONTROL_USER 0
#define FITSHOW_CONTROL_STOP 3
#define FITSHOW_CONTROL_PAUSE 6
#define FITSHOW_DATE_NOT_SUPPORT ""
#define FITSHOW_TREADMILL_SPORT_ID 0
#define FITSHOW_INFO_MODEL 0
#define FITSHOW_INFO_DATE 1
#define FITSHOW_INFO_SPEED 2
#define FITSHOW_INFO_INCLINE 3
#define FITSHOW_INFO_TOTAL 4
#define FITSHOW_INFO_UNKNOWN 5
#define FITSHOW_SYS_MODE_NORMAL 0
#define FITSHOW_SYS_MODE_TIMER 1
#define FITSHOW_SYS_MODE_DISTANCE 2
#define FITSHOW_SYS_MODE_CALORIE 3
#define FITSHOW_SYS_MODE_STEPS 4
#define FITSHOW_SYS_MODE_PROGRAMS 5
#define FITSHOW_SYS_MODE_MATCH 6
#define FITSHOW_STATUS_NORMAL 0
#define FITSHOW_STATUS_END 1
#define FITSHOW_STATUS_START 2
#define FITSHOW_STATUS_RUNNING 3
#define FITSHOW_STATUS_STOP 4
#define FITSHOW_STATUS_ERROR 5
#define FITSHOW_STATUS_SAFETY 6
#define FITSHOW_STATUS_STUDY 7
#define FITSHOW_STATUS_PAUSED 10
#define FITSHOW_DATA_SPORT 0
#define FITSHOW_DATA_INFO 1
#define FITSHOW_DATA_SPEED 2
#define FITSHOW_DATA_INCLINE 3
#define FITSHOW_DATA_CACHECACHE 4

class fitshowtreadmill : public treadmill {
    Q_OBJECT
  public:
    fitshowtreadmill(uint32_t poolDeviceTime = 200, bool noConsole = false, bool noHeartService = false,
                     double forceInitSpeed = 0.0, double forceInitInclination = 0.0);
    virtual ~fitshowtreadmill();
    bool connected() override;
    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;
    double minStepInclination() override;

  private:
    bool checkIncomingPacket(const uint8_t *data, uint8_t data_len) const;
    void forceSpeedOrIncline(double requestSpeed, double requestIncline);
    void btinit(bool startTape);
    void writeCharacteristic(const uint8_t *data, uint8_t data_len, const QString &info = QString());
    bool writePayload(const uint8_t *data, uint8_t data_len, const QString &info = QString());
    void scheduleWrite(const uint8_t *data, uint8_t data_len, const QString &info = QString());
    void startDiscover();
    void sendSportData();
    void removeFromBuffer();
    QBluetoothUuid serviceId;
    int retrySend = 0;
    bool noHeartService = false;
    bool anyrun = false;
    bool truetimer = false;
    uint32_t pollDeviceTime = 200;
    bool searchStopped = false;
    uint8_t firstInit = 0;
    double DistanceCalculated = 0;
    QByteArray lastPacket;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;
    int MAX_INCLINE = 30;
    int COUNTDOWN_VALUE = 0;
    int MAX_SPEED = 300;
    int MIN_INCLINE = 0;
    int MIN_SPEED = 0;
    int UNIT = -100;
    int SPORT_ID;
    int USER_ID;
    bool IS_PAUSE = false;
    bool IS_HRC = false;
    QString DEVICE_ID_NAME;
    int TOTAL = 0;
    QDate FACTORY_DATE;
    bool IS_STATUS_STUDY = false;
    bool IS_STATUS_ERRO = false;
    bool IS_STATUS_SAFETY = false;
    bool IS_RUNNING = false;
    int ERRNO = -1;
    int SYS_CONTROL_CMD = 0;
    int CURRENT_STATUS = 0;
    int INDOORRUN_MODE = 0;
    int INDOORRUN_CALORIE_DATA = 0;
    int INDOORRUN_TIME_DATA = 0;
    int INDOORRUN_PARAM_NUM = 0;
    int INDOORRUN_DISTANCE_DATA = 0;
    int RUN_WAY = 0;
    QStringList debugMsgs;
    QByteArray bufferWrite;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyService *gattCommunicationRSCService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    QDateTime lastChangedStepCount = QDateTime::currentDateTime();

    bool initDone = false;
    bool initRequest = false;

    double minStepInclinationValue = 1.0;
    bool noblepro_connected = false;
    bool fs_connected = false;

    metric rawInclination;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif
  Q_SIGNALS:
    void disconnected();
    void debug(QString string);
    void speedChanged(double speed);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void searchingStop();

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void changeInclinationRequested(double grade, double percentage);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // FITSHOWTREADMILL_H
