#ifndef KINGSMITHR2TREADMILL_H
#define KINGSMITHR2TREADMILL_H

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
#include <QMap>
#include <QObject>

#include "treadmill.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class kingsmithr2treadmill : public treadmill {

    Q_OBJECT
  public:
    kingsmithr2treadmill(uint32_t poolDeviceTime = 200, bool noConsole = false, bool noHeartService = false,
                         double forceInitSpeed = 0.0, double forceInitInclination = 0.0);
    bool connected() override;
    virtual bool canStartStop() override { return false; }

  private:
    const QByteArray PLAINTEXT_TABLE =
        QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=").toUtf8();
    const QByteArray ENCRYPT_TABLE =
        QStringLiteral("SaCw4FGHIJqLhN+P9RVTU/WcY6ObDdefgEijklmnopQrsBuvMxXz1yA2t5078KZ3=").toUtf8();
    const QByteArray ENCRYPT_TABLE_v2 =
        QStringLiteral("ZaCw4FGHIJqLhN+P9RMTU/WcY6ObDdefgEijklmnopQrsBuvVxXz1yA2t5078KS3=").toUtf8();
    const QByteArray ENCRYPT_TABLE_v3 =
        QStringLiteral("0aCw4FGHIJqLhN+P9RVTU/WcY6ObDdefgEijklmnopQrsBuvMxXz1yA2t5Z78KS3=").toUtf8();
    const QByteArray ENCRYPT_TABLE_v4 =
        QStringLiteral("ZaCw4FGHIJqLhN9P+RVTU/WcY6ObDdefgEijklmnopQrsBuvMxXz1yA2t5078KS3=").toUtf8();
    const QByteArray ENCRYPT_TABLE_v5 =
        QStringLiteral("iaCw4FGHIJqLhN+P9RVTU/WcY6ObDdefgEZjklmnopQrsBuvMxXz1yA2t5078KS3=").toUtf8();
    const QByteArray ENCRYPT_TABLE_v6 =
        QStringLiteral("ZaCw4FGHIJqLhN+P8RVTU/WcY6ObDdefgEijklmnopQrsBuvMxXz1yA2t5079KS3=").toUtf8();
    const QByteArray ENCRYPT_TABLE_v7 =
        QStringLiteral("baCw4FGHIJqLhN+P9RVTU/WcY6OZDdefgEijklmnopQrsBuvMxXz1yA2t5078KS3=").toUtf8();

    double GetInclinationFromPacket(const QByteArray &packet);
    double GetKcalFromPacket(const QByteArray &packet);
    double GetDistanceFromPacket(const QByteArray &packet);
    void forceSpeedOrIncline(double requestSpeed, double requestIncline);
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    void writeCharacteristic(const QString &data, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    bool noConsole = false;
    bool noHeartService = false;
    uint32_t pollDeviceTime = 200;
    bool searchStopped = false;
    uint8_t sec1Update = 0;
    uint8_t firstInit = 0;
    QMap<QString, double> props;
    QByteArray buffer;
    QByteArray lastValue;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;

    enum KINGSMITH_R2_CONTROL_MODE { AUTOMODE = 0, MANUAL, STANDBY, UNKNOWN_CONTROL_MODE };
    enum KINGSMITH_R2_RUN_STATE { STOP = 0, START, UNKNOWN_RUN_STATE };
    KINGSMITH_R2_CONTROL_MODE lastControlMode = UNKNOWN_CONTROL_MODE;
    KINGSMITH_R2_RUN_STATE lastRunState = UNKNOWN_RUN_STATE;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    bool initDone = false;
    bool initRequest = false;

    bool KS_NACH_X21C = false;
    bool KS_NACH_X21C_2 = false;
    bool KS_HDSY_X21C = false;
    bool KS_HDSY_X21C_2 = false;
    bool KS_NGCH_G1C = false;
    bool KS_NGCH_G1C_2 = false;
    bool KS_NACH_MXG = false;

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

#endif // KINGSMITHR2TREADMILL_H
