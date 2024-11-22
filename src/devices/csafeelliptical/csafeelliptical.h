#ifndef CSAFEELLIPTICAL_H
#define CSAFEELLIPTICAL_H

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
#include "devices/csafe/csaferunner.h"
#include "devices/csafe/csafeutility.h"
#include "devices/csafe/serialhandler.h"
// #include "serialport.h"

#include "devices/elliptical.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
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

// #include "serialport.h"

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

/*
class csafeellipticalThread : public QThread {
    Q_OBJECT

  public:
    explicit csafeellipticalThread();

    void run();

  signals:
    void onDebug(QString debug);
    void onPower(double power);
    void onCadence(double cadence);
    void onHeart(double hr);
    void onCalories(double calories);
    void onDistance(double distance);
    void onPace(double pace);
    void onStatus(char status);
    void onSpeed(double speed);
    void portavailable(bool available);
    void onCsafeFrame(const QVariantMap &frame);

  private:
    QString deviceFilename;

#ifdef Q_OS_ANDROID
    QList<jbyte> bufRX;
    bool cleanFrame = false;
#endif
};

*/

class csafeelliptical : public elliptical {
    Q_OBJECT
  public:
    csafeelliptical(bool noWriteResistance, bool noHeartService, bool noVirtualDevice, int8_t bikeResistanceOffset,
                    double bikeResistanceGain);
    bool connected() override;

  private:
    QTimer *refresh;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    int lastStatus = -1;
    QSettings settings;

    uint16_t watts() override;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noVirtualDevice = false;

    bool distanceIsChanging = false;
    metric distanceReceived;

    bool _connected = true;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);
    void sendCsafeCommand(const QStringList &commands);

  private slots:
    void update();
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void changeInclinationRequested(double grade, double percentage);
    void changeResistance(resistance_t res) override;
    void onPower(double power);
    void onCadence(double cadence);
    void onHeart(double hr);
    void onCalories(double calories);
    void onDistance(double distance);
    void onStatus(char status);
    void onSpeed(double speed);
    void portAvailable(bool available);
    void onCsafeFrame(const QVariantMap &frame);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
};

#endif // CSAFEELLIPTICAL_H
