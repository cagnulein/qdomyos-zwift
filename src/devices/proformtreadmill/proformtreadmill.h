#ifndef PROFORMTREADMILL_H
#define PROFORMTREADMILL_H

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

class proformtreadmill : public treadmill {
    Q_OBJECT
  public:
    proformtreadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    double minStepSpeed() override { return 0.1; }

  private:
    double GetDistanceFromPacket(QByteArray packet);
    QTime GetElapsedFromPacket(QByteArray packet);
    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    void sendPoll();
    void forceIncline(double incline);
    void forceSpeed(double speed);

    QTimer *refresh;
    uint8_t counterPoll = 0;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    bool nordictrack10 = false;
    bool nordictrackt70 = false;
    bool nordictrack_t65s_treadmill = false;
    bool nordictrack_treadmill_ultra_le = false;
    bool proform_treadmill_carbon_tls = false;
    bool nordictrack_s30_treadmill = false;
    bool proform_treadmill_1800i = false;
    bool proform_treadmill_se = false;
    bool proform_treadmill_8_0 = false;
    bool proform_treadmill_9_0 = false;
    bool proform_cadence_lt = false;
    bool norditrack_s25i_treadmill = false;
    bool norditrack_s25_treadmill = false;
    bool nordictrack_t65s_83_treadmill = false;
    bool nordictrack_incline_trainer_x7i = false;
    bool proform_treadmill_z1300i = false;
    bool proform_pro_1000_treadmill = false;
    bool nordictrack_s20_treadmill = false;
    bool nordictrack_s20i_treadmill = false;
    bool proform_treadmill_l6_0s = false;
    bool proform_8_5_treadmill = false;
    bool proform_2000_treadmill = false;
    bool proform_treadmill_sport_8_5 = false;
    bool proform_treadmill_505_cst = false;
    bool proform_treadmill_705_cst = false;
    bool proform_carbon_tl = false;
    bool proform_proshox2 = false;
    bool proform_595i_proshox2 = false;
    bool proform_treadmill_8_7 = false;
    bool proform_treadmill_705_cst_V78_239 = false;
    bool nordictrack_treadmill_exp_5i = false;
    bool proform_carbon_tl_PFTL59720 = false;
    bool proform_treadmill_sport_70 = false;
    bool proform_treadmill_575i = false;
    bool proform_performance_300i = false;
    bool proform_performance_400i = false;
    bool proform_treadmill_c700 = false;
    bool proform_treadmill_c960i = false;
    bool nordictrack_tseries5_treadmill = false;
    bool proform_carbon_tl_PFTL59722c = false;
    bool proform_treadmill_1500_pro = false;
    bool proform_505_cst_80_44 = false;
    bool proform_trainer_8_0 = false;
    bool proform_treadmill_705_cst_V80_44 = false;
    bool nordictrack_t65s_treadmill_81_miles = false;
    bool nordictrack_elite_800 = false;
    bool proform_treadmill_995i = false;
    bool nordictrack_series_7 = false;

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
    void changeInclinationRequested(double grade, double percentage);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // PROFORMTREADMILL_H
