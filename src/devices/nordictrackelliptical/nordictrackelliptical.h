#ifndef NORDITRACKELLIPTICAL_H
#define NORDITRACKELLIPTICAL_H

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

#include "devices/elliptical.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class nordictrackelliptical : public elliptical {
    Q_OBJECT
  public:
    nordictrackelliptical(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                          double bikeResistanceGain);
    bool connected() override;
    int pelotonToEllipticalResistance(int pelotonResistance) override;
    bool inclinationAvailableByHardware()  override { 
      if(nordictrack_elliptical_c7_5 || nordictrack_se7i) 
        return true; 
      else 
        return false; 
    }

  private:
    double GetDistanceFromPacket(QByteArray packet);
    QTime GetElapsedFromPacket(QByteArray packet);
    double GetResistanceFromPacket(QByteArray packet);
    double GetInclinationFromPacket(QByteArray packet);
    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void forceResistance(resistance_t requestResistance);
    void startDiscover();
    void sendPoll();
    void forceIncline(double incline);
    void forceSpeed(double speed);

    QTimer *refresh;
    uint8_t counterPoll = 0;
    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    resistance_t max_resistance = 20;
    double max_inclination = 0;
    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastSpeedChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool nordictrack_elliptical_c7_5 = false;
    bool nordictrack_se7i = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void changeInclinationRequested(double grade, double percentage);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // NORDITRACKELLIPTICAL_H
