#ifndef VIRTUALTREADMILL_H
#define VIRTUALTREADMILL_H

#include <QObject>

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
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

#include <QtCore/qlist.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include "dirconmanager.h"
#include "treadmill.h"

class virtualtreadmill : public QObject {
    Q_OBJECT
  public:
    virtualtreadmill(bluetoothdevice *t, bool noHeartService);
    bool connected();
    bool autoInclinationEnabled() { return m_autoInclinationEnabled; }

  private:
    QLowEnergyController *leController = nullptr;
    QLowEnergyService *serviceFTMS = nullptr;
    QLowEnergyService *serviceRSC = nullptr;
    QLowEnergyService *serviceHR = nullptr;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceDataFTMS;
    QLowEnergyServiceData serviceDataRSC;
    QLowEnergyServiceData serviceDataHR;
    QTimer treadmillTimer;
    bluetoothdevice *treadMill;

    uint64_t lastSlopeChanged = 0;

    CharacteristicWriteProcessor2AD9 *writeP2AD9 = 0;
    CharacteristicNotifier2AD2 *notif2AD2 = 0;
    CharacteristicNotifier2AD9 *notif2AD9 = 0;
    CharacteristicNotifier2A53 *notif2A53 = 0;
    CharacteristicNotifier2ACD *notif2ACD = 0;
    CharacteristicNotifier2A37 *notif2A37 = 0;
    DirconManager *dirconManager = 0;

    bool noHeartService = false;

    bool m_autoInclinationEnabled = false;

    bool ftmsServiceEnable();
    bool ftmsTreadmillEnable();
    bool RSCEnable();

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void debug(QString string);
    void changeInclination(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void treadmillProvider();
    void reconnect();
    void slopeChanged();
    void dirconChangedInclination(double grade, double percentage);
};

#endif // VIRTUALTREADMILL_H
