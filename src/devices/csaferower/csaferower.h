#ifndef CSAFEROWER_H
#define CSAFEROWER_H

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

#include "devices/csafe/csafe.h"
#include "devices/rower.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QSettings>
#include <QThread>

#ifdef WIN32
#include <windows.h>

#include <winbase.h>
#else
#include <sys/ioctl.h>
#include <termios.h> // unix!!
#include <unistd.h>  // unix!!
#ifndef N_TTY        // for OpenBSD, this is a hack XXX
#define N_TTY 0
#endif
#endif

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QAndroidJniObject>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

/* read timeouts in microseconds */
#define CT_READTIMEOUT 1000
#define CT_WRITETIMEOUT 2000

class csaferowerThread : public QThread {
    Q_OBJECT

  public:
    explicit csaferowerThread();

    void run();

  signals:
    void onDebug(QString debug);
    void newPacket(QByteArray p);
    void onPower(double power);
    void onCadence(double cadence);
    void onHeart(double hr);
    void onCalories(double calories);
    void onDistance(double distance);
    void onPace(double pace);
    void onStatus(char status);

  private:
    // Utility and BG Thread functions
    int openPort();
    int closePort();

    // Mutex for controlling accessing private data
    QMutex pvars;

    // device port
    QString deviceFilename;
#ifdef WIN32
    HANDLE devicePort;  // file descriptor for reading from com3
    DCB deviceSettings; // serial port settings baud rate et al
#else
    int devicePort;                // unix!!
    struct termios deviceSettings; // unix!!
#endif
    // raw device utils
    int rawWrite(uint8_t *bytes, int size); // unix!!
    int rawRead(uint8_t *bytes, int size);  // unix!!

#ifdef Q_OS_ANDROID
    QList<jbyte> bufRX;
    bool cleanFrame = false;
#endif
};

class csaferower : public rower {
    Q_OBJECT
  public:
    csaferower(bool noWriteResistance, bool noHeartService, bool noVirtualDevice);
    bool connected() override;

  private:
    QTimer *refresh;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    uint16_t watts() override;

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
    void newPacket(QByteArray p);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void onPower(double power);
    void onCadence(double cadence);
    void onHeart(double hr);
    void onCalories(double calories);
    void onDistance(double distance);
    void onPace(double pace);
    void onStatus(char status);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
};

#endif // CSAFEROWER_H
