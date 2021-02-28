#ifndef M3IBIKEBIKE_H
#define M3IBIKEBIKE_H

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>
#include <QtCore/qmutex.h>

#include <QObject>
#include <QString>
#include <QDateTime>

#include "virtualbike.h"
#include "bike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

typedef struct keiser_m3i_out_s {
    int time = 0;
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
    void inner_reset(int buffSize);
    bool inner_step(keiser_m3i_out_t * f);
private:
    #define M3I_EQUAL_TIME_THRESHOLD 8
    #define M3I_VALID_PULSE_THRESHOLD 50
    #define M3I_PAUSE_DELAY_DETECT_THRESHOLD 10000
    int buffSize = 150;
    double * dist_buff = 0;
    int * dist_buff_time = 0;
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

    void _set_offsets();

    bool step_cyc(keiser_m3i_out_t * f, qint64 now);

    void fillTimeRFields(keiser_m3i_out_t * f, qint64 updateTime);

    double calcSpeed(keiser_m3i_out_t * f, bool pause);

    bool inPause() const;

    void detectPause(const keiser_m3i_out_t * f);
};

class m3ibike : public bike {
    Q_OBJECT
public:
    m3ibike(bool noWriteResistance, bool noHeartService);
    virtual ~m3ibike();
    bool connected();

    void* VirtualBike();
    void* VirtualDevice();
    static bool parse_data(const QByteArray& data, keiser_m3i_out_t * f);
    static bool valid_id(int id);
private:
    uint16_t watts();
    QTimer* detectDisc = 0;
    KeiserM3iDeviceSimulator k3s;
    keiser_m3i_out_t k3;

    virtualbike* virtualBike = 0;

    bool firstUpdate = true;
    uint16_t m_watts = 0;

    bool initDone = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

#ifdef Q_OS_IOS
    lockscreen* h = 0;
#endif

signals:
    void disconnected();
    void debug(QString string);

public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
};


#endif // M3IBIKEBIKE_H
