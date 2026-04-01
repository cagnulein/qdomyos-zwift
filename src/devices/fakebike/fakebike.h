#ifndef FAKEBIKE_H
#define FAKEBIKE_H


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

#include "devices/bike.h"
#include "ergtable.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class fakebike : public bike {
    Q_OBJECT
  public:
    fakebike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice);
    bool connected() override;
    uint16_t watts() override;
    resistance_t maxResistance() override { return 100; }
    resistance_t resistanceFromPowerRequest(uint16_t power) override;
    double maxGears() override;
    double minGears() override;
    
  private:
    QTimer *refresh;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool noVirtualDevice = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;    

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

    uint16_t wattsFromResistance(double resistance);

  signals:
    void disconnected();
    void debug(QString string);

  private slots:
    void changeInclinationRequested(double grade, double percentage);
    void update();
    
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};
#endif // FAKEBIKE_H
