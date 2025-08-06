#ifndef TRAINPROGRAM_H
#define TRAINPROGRAM_H
#include "bluetooth.h"
#include <QGeoCoordinate>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QTime>
#include <QTimer>

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

#include "zwift-api/PlayerStateWrapper.h"
#include "zwift-api/zwift_client_auth.h"

#ifdef Q_CC_MSVC
#include "zwift-api/zwift_messages.pb.h"
#endif

class trainrow {
  public:
    QTime duration = QTime(0, 0, 0, 0);
    QDateTime started = QDateTime();
    QDateTime ended = QDateTime();
    double distance = -1;
    double speed = -1;
    double lower_speed = -1;   // used for peloton
    double average_speed = -1; // used for peloton
    double upper_speed = -1;   // used for peloton
    double fanspeed = -1;
    double inclination = -200;
    double lower_inclination = -200;   // used for peloton
    double average_inclination = -200; // used for peloton
    double upper_inclination = -200;   // used for peloton
    resistance_t resistance = -1;
    resistance_t lower_resistance = -1;
    resistance_t average_resistance = -1; // used for peloton
    resistance_t upper_resistance = -1;
    int8_t requested_peloton_resistance = -1;
    int8_t lower_requested_peloton_resistance = -1;
    int8_t average_requested_peloton_resistance = -1; // used for peloton
    int8_t upper_requested_peloton_resistance = -1;
    int8_t pace_intensity = -1; // used for peloton
    int16_t cadence = -1;
    int16_t lower_cadence = -1;
    int16_t average_cadence = -1; // used for peloton
    int16_t upper_cadence = -1;
    bool forcespeed = false;
    int8_t loopTimeHR = 10;
    int8_t zoneHR = -1;
    int16_t HRmin = -1;
    int16_t HRmax = -1;
    double maxSpeed = -1;
    double minSpeed = -1;
    int8_t maxResistance = -1;
    int32_t power = -1;
    int32_t mets = -1;
    QTime rampDuration = QTime(0, 0, 0, 0); // QZ split the ramp in 1 second segments. This field will tell you how long
                                            // is the ramp from this very moment
    QTime rampElapsed = QTime(0, 0, 0, 0);
    QTime gpxElapsed = QTime(0, 0, 0, 0);
    double latitude = NAN;
    double longitude = NAN;
    double altitude = NAN;
    double azimuth = NAN;
    QString toString() const;
};

class trainprogram : public QObject {
    Q_OBJECT

  public:
    trainprogram(const QList<trainrow> &, bluetooth *b, QString *description = nullptr, QString *tags = nullptr,
                 bool videoAvailable = false);
    void save(const QString &filename);
    static trainprogram *load(const QString &filename, bluetooth *b, QString Extension);
    static QList<trainrow> loadXML(const QString &filename, bluetoothdevice::BLUETOOTH_TYPE device_type);
    static bool saveXML(const QString &filename, const QList<trainrow> &rows);
    static bool hasTargetPower(const QString &filename);
    QTime totalElapsedTime();
    QTime currentRowElapsedTime();
    QTime currentRowRemainingTime();
    QTime remainingTime();
    double currentTargetMets();
    QTime duration();
    double totalDistance();
    trainrow currentRow();
    trainrow getRowFromCurrent(uint32_t offset);
    void increaseElapsedTime(int32_t i);
    void decreaseElapsedTime(int32_t i);
    int32_t offsetElapsedTime() { return offset; }
    void clearRows();
    double avgSpeedFromGpxStep(int gpxStep, int seconds);
    double TimeRateFromGPX(double gpxsecs, double videosecs, double currentspeed, int recordingFactor);
    int TotalGPXSecs();
    double weightedInclination(int step);
    double medianInclination(int step);
    bool overridePowerForCurrentRow(double power);
    bool overrideZoneHRForCurrentRow(uint8_t zone);
    bool powerzoneWorkout() {
        foreach(trainrow r, rows) {
            if(r.power != -1) return true;
        }
        return false;
    }

    QList<trainrow> rows;
    QList<trainrow> loadedRows; // rows as loaded
    QString description = "";
    QString tags = "";
    bool enabled = true;
    bool videoAvailable = false;
    void setVideoAvailable(bool v) {videoAvailable = v;}

    void restart();
    bool isStarted() { return started; }
    void scheduler(int tick);

    void applySpeedFilter();

  public slots:
    void onTapeStarted();
    void scheduler();

private slots:
    void pelotonOCRprocessPendingDatagrams();

  signals:
    void start();
    void stop(bool paused);
    void lap();
    void changeSpeed(double speed);
    bool changeFanSpeed(uint8_t speed);
    void changeInclination(double grade, double inclination);
    void changeNextInclination300Meters(QList<MetersByInclination>);
    void changeResistance(resistance_t resistance);
    void changeRequestedPelotonResistance(int8_t resistance);
    void changeCadence(int16_t cadence);
    void changePower(int32_t power);
    void changeSpeedAndInclination(double speed, double inclination);
    void changeGeoPosition(QGeoCoordinate p, double azimuth, double avgAzimuthNext300Meters);
    void changeTimestamp(QTime source, QTime actual);
    void toastRequest(QString message);
    void zwiftLoginState(bool ok);

  private:
    void end();
    mutable QRecursiveMutex schedulerMutex;
    double avgAzimuthNext300Meters();
    QList<MetersByInclination> inclinationNext300Meters();
    QList<MetersByInclination> avgInclinationNext300Meters();
    double avgInclinationNext100Meters(int step);
    uint32_t calculateTimeForRow(int32_t row);
    uint32_t calculateTimeForRowMergingRamps(int32_t row);
    double calculateDistanceForRow(int32_t row);
    bluetooth *bluetoothManager;
    bool started = false;
    int32_t ticks = 0;
    uint16_t currentStep = 0;
    int32_t offset = 0;
    double lastOdometer = 0;
    double currentStepDistance = 0;
    QTimer timer;
    double lastGpxRateSetAt = 0.0;
    double lastGpxRateSet = 0.0;
    double lastGpxSpeedSet = 0.0;
    int lastStepTimestampChanged = 0;
    double lastCurrentStepDistance = 0.0;
    QTime lastCurrentStepTime = QTime(0, 0, 0);

    QUdpSocket* pelotonOCRsocket = nullptr;
    void pelotonOCRcomputeTime(QString t);
    
    AuthToken* zwift_auth_token = nullptr;
    World* zwift_world = nullptr;
    int zwift_player_id = -1;
    
#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

};

#endif // TRAINPROGRAM_H
