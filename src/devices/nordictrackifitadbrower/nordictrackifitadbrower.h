#ifndef NORDICTRACKIFITADBROWER_H
#define NORDICTRACKIFITADBROWER_H

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

#include "devices/rower.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class nordictrackifitadbrowerLogcatAdbThread : public QThread {
    Q_OBJECT

  public:
    explicit nordictrackifitadbrowerLogcatAdbThread(QString s);
    bool runCommand(QString command);

    void run() override;

  signals:
    void onSpeedResistance(double speed, double resistance);
    void debug(QString message);
    void onWatt(double watt);
    void onHRM(int hrm);
    void onCadence(double cadence);

  private:
    QString adbCommandPending = "";
    QString runAdbCommand(QString command);
    double speed = 0;
    double resistance = 0;
    double cadence = 0;
    double watt = 0;
    int hrm = 0;
    QString name;
    struct adbfile {
        QDateTime date;
        QString name;
    };

    void runAdbTailCommand(QString command);
};

class nordictrackifitadbrower : public rower {
    Q_OBJECT
  public:
    nordictrackifitadbrower(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                           double bikeResistanceGain);
    bool connected() override;

  private:
    const resistance_t max_resistance = 32; // max resistance for rower
    void forceResistance(double resistance);
    uint16_t watts() override;
    double getDouble(QString v);
    uint16_t wattsFromResistance(double resistance, double cadence);

    QTimer *refresh;

    uint8_t sec1Update = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastResistanceChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;
    bool cadenceReadFromTM = false;
    bool resistanceReadFromTM = false;
    bool wattReadFromTM = false;
    bool speedReadFromTM = false;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    bool gearsAvailable = false;

    QUdpSocket *socket = nullptr;
    QHostAddress lastSender;

    nordictrackifitadbrowerLogcatAdbThread *logcatAdbThread = nullptr;

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
    void changeResistanceRequested(double resistance);
    void onHRM(int hrm);
    void onWatt(double watt);
    void onCadence(double cadence);
    void onSpeedResistance(double speed, double resistance);

    void update();
};

#endif // NORDICTRACKIFITADBROWER_H