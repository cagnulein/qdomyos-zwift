#ifndef NORDICTRACKIFITADBTREADMILL_H
#define NORDICTRACKIFITADBTREADMILL_H

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
#include <QThread>
#include <QUdpSocket>
#include <QRect>
#include <QRegularExpression>
#include "treadmill.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class nordictrackifitadbtreadmillLogcatAdbThread : public QThread {
    Q_OBJECT

  public:
    explicit nordictrackifitadbtreadmillLogcatAdbThread(QString s);
    bool runCommand(QString command);

    void run() override;
    bool stop = false;    

  signals:
    void onSpeedInclination(double speed, double inclination);
    void debug(QString message);
    void onWatt(double watt);
    void onCadence(double cadence);

  private:
    QString adbCommandPending = "";
    double speed = 0;
    double inclination = 0;
    double watt = 0;
    double cadence = 0;
    QString name;
    struct adbfile {
        QDateTime date;
        QString name;
    };

    QString runAdbCommand(QString command);
    void runAdbTailCommand(QString command);
};

class nordictrackifitadbtreadmill : public treadmill {
    Q_OBJECT
  public:
    enum RequestOrigin {
        ORIGIN_USER,     // User initiated action
        ORIGIN_GRPC,     // Action from gRPC/iFit app
        ORIGIN_INTERNAL  // Internal app logic
    };

    nordictrackifitadbtreadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    bool canStartStop() override;
    double minStepSpeed() override { return 0.1; }
    bool changeFanSpeed(uint8_t speed) override;

  private:
    struct DisplayValue {
        QString value;
        QString label;
        QRect rect;
    };

    void forceIncline(double incline);
    void forceSpeed(double speed);
    double getDouble(QString v);
    void initiateThreadStop();
    
    // gRPC integration methods
    void initializeGrpcService();
    void startGrpcMetricsUpdates();
    void stopGrpcMetricsUpdates();
    double getGrpcSpeed();
    double getGrpcIncline();
    double getGrpcWatts();
    double getGrpcCadence();
    double getGrpcHeartRate();
    void setGrpcSpeed(double speed);
    void setGrpcFanSpeed(int fanSpeed);
    int getGrpcFanSpeed();
    void setGrpcIncline(double incline);
    void startGrpcWorkoutStateMonitoring();
    int getGrpcWorkoutState();

    QTimer *refresh;

    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastInclinationChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;
    bool wattReadFromTM = false;
    bool cadenceReadFromTM = false;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool grpcInitialized = false;
    int previousWorkoutState = 1; // WORKOUT_STATE_IDLE
    bool proform_trainer_9_0 = false;

    QDateTime lastTimeDataReceived;
    bool firstDataReceived = true;

    // Track origin of stop/pause/start requests to prevent echoing gRPC events
    RequestOrigin requestStartOrigin = ORIGIN_USER;
    RequestOrigin requestStopOrigin = ORIGIN_USER;
    RequestOrigin requestPauseOrigin = ORIGIN_USER;

    QUdpSocket *socket = nullptr;
    QHostAddress lastSender;

#ifdef Q_OS_WIN32
    nordictrackifitadbtreadmillLogcatAdbThread *logcatAdbThread = nullptr;
#endif

    DisplayValue extractValue(const QString& ocrText, int imageWidth, bool isLeftSide);
    void processOCROutput(const QString& ocrText, int imageWidth);

    int x14i_inclination_lookuptable(double reqInclination);
    int proform_trainer_9_0_speed_lookuptable(double reqSpeed);
    int proform_trainer_9_0_inclination_lookuptable(double reqInclination);

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

    QString lastCommand = "";

  signals:
    void disconnected();
    void debug(QString string);

  private slots:

    void onSpeedInclination(double speed, double inclination);
    void onWatt(double watt);
    void onCadence(double cadence);

    void processPendingDatagrams();
    void changeInclinationRequested(double grade, double percentage);

    void update();
    
  public slots:
    void stopLogcatAdbThread();
};

#endif // NORDICTRACKIFITADBTREADMILL_H
