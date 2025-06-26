#ifndef NORDICTRACKIFITADBBIKE_H
#define NORDICTRACKIFITADBBIKE_H

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

#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QtAndroid>
#endif

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class nordictrackifitadbbikeLogcatAdbThread : public QThread {
    Q_OBJECT

  public:
    explicit nordictrackifitadbbikeLogcatAdbThread(QString s);    
    bool runCommand(QString command);

    void run() override;

  signals:
    void onSpeedInclination(double speed, double inclination);
    void debug(QString message);
    void onWatt(double watt);
    void onHRM(int hrm);

  private:
    QString adbCommandPending = "";
    QString runAdbCommand(QString command);
    double speed = 0;
    double inclination = 0;
    double watt = 0;
    int hrm = 0;
    QString name;
    struct adbfile {
        QDateTime date;
        QString name;
    };

    void runAdbTailCommand(QString command);
};

class nordictrackifitadbbike : public bike {
    Q_OBJECT
  public:
    nordictrackifitadbbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain);
    bool connected() override;
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    bool inclinationAvailableByHardware() override;
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    bool ifitCompatible() override;
    void changePower(int32_t power) override;
    bool changeFanSpeed(uint8_t speed) override;

  private:
    const resistance_t max_resistance = 20; // max inclination for s22i
    void forceResistance(double resistance);
    uint16_t watts() override;
    double getDouble(QString v);
    uint16_t wattsFromResistance(double inclination, double cadence);
    double bikeResistanceToPeloton(resistance_t resistance);
    
    // gRPC integration methods
    void initializeGrpcService();
    void startGrpcMetricsUpdates();
    void stopGrpcMetricsUpdates();
    double getGrpcSpeed();
    double getGrpcIncline();
    double getGrpcWatts();
    double getGrpcCadence();
    double getGrpcResistance();
    void setGrpcResistance(double resistance);
    void setGrpcIncline(double inclination);
    void setGrpcWatts(double watts);
    void disableGrpcWatts();
    void setGrpcFanSpeed(int fanSpeed);
    int getGrpcFanSpeed();

    QTimer *refresh;

    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastInclinationChanged = QDateTime::currentDateTime();
    QDateTime lastGrpcInclinationChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool grpcInitialized = false;
    bool lastErgMode = true;
    bool hasActiveWattsTarget = false;

    bool gearsAvailable = false;

    QUdpSocket *socket = nullptr;
    QHostAddress lastSender;

    nordictrackifitadbbikeLogcatAdbThread *logcatAdbThread = nullptr;

    QString lastCommand;

    QString ip;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  private slots:

    void processPendingDatagrams();
    void changeInclinationRequested(double grade, double percentage);
    void onHRM(int hrm);

    void update();
};

#endif // NORDICTRACKIFITADBBIKE_H
