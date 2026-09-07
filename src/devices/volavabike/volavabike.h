#ifndef VOLAVABIKE_H
#define VOLAVABIKE_H

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

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

// Protocol constants
// Service:  00010203-0405-0607-0809-0a0b0c0d1910
// Notify:   00010203-0405-0607-0809-0a0b0c0d2b10  (device → phone)
// Write:    00010203-0405-0607-0809-0a0b0c0d2b11  (phone → device, WriteNoResponse)
//
// Workout frame (16 bytes, every ~225 ms):
//   AA 10 00 80 04 01 PH PL 04 02 00 CC 02 03 XX CS
//   PH:PL = instant power in watts (16-bit BE)
//   CC    = cadence in RPM
//   XX    = unknown (possible grip HR)
//   CS    = (sum of all bytes) & 0xFF

class volavabike : public bike {
    Q_OBJECT
  public:
    volavabike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
               double bikeResistanceGain);
    bool connected() override;

  private:
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info,
                             bool disable_log = false, bool wait_for_response = false);
    void btinit();
    uint16_t watts() override;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    bool initDone = false;
    bool initRequest = false;
    bool noWriteResistance = false;
    bool noHeartService = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  Q_SIGNALS:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                               const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                               const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // VOLAVABIKE_H
