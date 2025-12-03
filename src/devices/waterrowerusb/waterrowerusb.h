#ifndef WATERROWERUSB_H
#define WATERROWERUSB_H

#include <QBluetoothDeviceDiscoveryAgent>
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
#include <QThread>

#include <QDateTime>
#include <QObject>
#include <QString>

#include "devices/rower.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class waterrowerusbThread : public QThread {
    Q_OBJECT

public:
    explicit waterrowerusbThread(QObject *parent = nullptr);
    void run() override;
    void stop();

signals:
    void onDebug(QString debug);
    void onConnected();
    void onDisconnected();
    void onError(QString error);
    void onStroke(double strokeRate, double distance, double pace, double watts, double calories);

private:
    bool running = false;
    QMutex mutex;

#ifdef Q_OS_ANDROID
    void initializeWaterRower();
    void shutdownWaterRower();
    void processWaterRowerData();
#endif
};

class waterrowerusb : public rower {
    Q_OBJECT
public:
    waterrowerusb(bool noWriteResistance, bool noHeartService, bool noVirtualDevice);
    ~waterrowerusb() override;
    bool connected() override;

private:
    QTimer *refresh;
    waterrowerusbThread *workerThread;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    uint16_t watts() override;
    void updateDisplay(uint16_t elapsed);

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noVirtualDevice = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;

    bool distanceIsChanging = false;
    metric distanceReceived;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

signals:
    void disconnected();
    void debug(QString string);

private slots:
    void update();
    void onWaterRowerConnected();
    void onWaterRowerDisconnected(); 
    void onWaterRowerError(QString error);
    void onWaterRowerStroke(double strokeRate, double distance, double pace, double watts, double calories);

public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
};

#endif // WATERROWERUSB_H