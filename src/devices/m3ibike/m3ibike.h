#ifndef M3IBIKEBIKE_H
#define M3IBIKEBIKE_H

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
#include <QJniObject>
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
#include "ios/M3iIOS-Interface.h"
#include "ios/lockscreen.h"
#endif

typedef struct keiser_m3i_out_s {
    int time = 0;
    int time_orig = 0;
    int timeR = 0;
    int timeRms = 0;
    int timeRAbsms = 0;
    double distance = 0.0;
    double distanceR = 0.0;
    int calorie = 0;
    double speed = 0.0;
    int pulse = 0;
    int rpm = 0;
    int watt = 0;
    int incline = 0;
    double pulseMn = 0.0;
    double rpmMn = 0.0;
    double speedMn = 0.0;
    double wattMn = 0.0;
    int software = 0;
    int firmware = 0;
    int system_id = 0;
    void reset() {
        time_orig = 0;
        time = 0;
        timeR = 0;
        timeRms = 0;
        timeRAbsms = 0;
        distance = 0.0;
        distanceR = 0.0;
        calorie = 0;
        speed = 0.0;
        pulse = 0;
        rpm = 0;
        watt = 0;
        incline = 0;
        pulseMn = 0.0;
        rpmMn = 0.0;
        speedMn = 0.0;
        wattMn = 0.0;
        software = 0;
        firmware = 0;
        system_id = 0;
    }
} keiser_m3i_out_t;

class KeiserM3iDeviceSimulator {
  public:
    virtual ~KeiserM3iDeviceSimulator();
    KeiserM3iDeviceSimulator();
    void inner_reset(int buffSize, int equalTimeDist);
    bool inner_step(keiser_m3i_out_t *f);

  private:
#define M3I_EQUAL_TIME_THRESHOLD 8
#define M3I_VALID_PULSE_THRESHOLD 50
#define M3I_PAUSE_DELAY_DETECT_THRESHOLD 10000
    int equalTimeDistanceThreshold = 2500;
    int buffSize = 150;
    double *dist_buff = 0;
    int *dist_buff_time = 0;
    int dist_buff_idx = 0;
    int dist_buff_size = 0;
    double dist_acc = 0.0;
    double old_dist = -1.0;
    double timeRms_acc = 0;
    int old_timeRms = 0;
    int time_o = 0;
    int time_old = 0;
    int calorie_o = 0;
    int calorie_old = 0;
    int distance_o = 0.0;
    int distance_old = 0.0;
    int nPulses = 0;
    int sumWatt = 0;
    int sumTime = 0;
    double sumSpeed = 0.0;
    int sumPulse = 0;
    int sumRpm = 0;
    qint64 lastUpdatePostedTime = 0;
    int equalTime = 0;
    int old_time_orig = -1;
    int nActiveUpdates = 0;
    qint64 sessionStart = 0;
    qint64 lastUpdateTime = 0;
    qint64 equalTimeDistance = 0;
    bool oldPause = true;

    void _set_offsets();

    bool step_cyc(keiser_m3i_out_t *f, qint64 now);

    void fillTimeRFields(keiser_m3i_out_t *f, qint64 updateTime);

    double calcSpeed(keiser_m3i_out_t *f, bool pause);

    bool inPause(qint64 ud) const;

    void detectPause(const keiser_m3i_out_t *f, qint64 ud);
};

#define M3i_DISCONNECT_THRESHOLD 5s
class m3ibike : public bike {
    Q_OBJECT
  public:
    m3ibike(bool noWriteResistance, bool noHeartService);
    virtual ~m3ibike();
    bool connected() override;
    static bool parse_data(const QByteArray &data, keiser_m3i_out_t *f);
    static bool valid_id(int id);
    static bool isCorrectUnit(const QBluetoothDeviceInfo &device);
    void disconnectBluetooth();
  signals:
    void disconnected();
    void debug(QString string);
  public slots:
    void searchingStop();
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private:
    bool heartRateBeltDisabled = true;
    bool antHeart = false;
    bool disconnecting = false;
    void initScan();
    Q_INVOKABLE void processAdvertising(const QByteArray &data);
    Q_INVOKABLE void restartScan();
    uint16_t watts() override;
    QTimer *detectDisc = nullptr, *elapsedTimer = nullptr;
    KeiserM3iDeviceSimulator k3s;
    keiser_m3i_out_t k3;
    qint64 lastTimerRestart = -1;
    int lastTimerRestartOffset = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    bool firstUpdate = true;

    bool initDone = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
#if defined(Q_OS_ANDROID)
    QJniObject bluetoothAdapter;
    QJniObject bluetoothScanner;
    QJniObject scanCallback;
    QJniObject listOfFilters;
    QJniObject settingsObject;
    QJniObject filterObject0;
    bool scannerActive = false;
    static void newAndroidScanResult(JNIEnv *env, jobject /*thiz*/, jobject record);
    static void newAndroidScanError(JNIEnv *, jobject /*thiz*/, jint code);
#endif
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = nullptr;
#ifdef Q_OS_IOS
    M3iIOS *m3iIOS = 0;
    lockscreen *h = 0;
    m3i_result_t m3i_ios_result;
#endif
#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID)
    bool qt_search;
#endif
  private slots:
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    void deviceUpdatedPriv(const QBluetoothDeviceInfo &, QBluetoothDeviceInfo::Fields);
    void deviceDiscoveredPriv(const QBluetoothDeviceInfo &device);
#endif
    void discoveryFinishedPriv();
};

#endif // M3IBIKEBIKE_H
