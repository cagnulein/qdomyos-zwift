#ifndef OCTANEELLIPTICAL_H
#define OCTANEELLIPTICAL_H

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

#include "devices/elliptical.h"
#include "virtualdevices/virtualtreadmill.h"

class octaneelliptical : public elliptical {
    Q_OBJECT
  public:
    octaneelliptical(uint32_t poolDeviceTime = 200, bool noConsole = false, bool noHeartService = false,
                     double forceInitSpeed = 0.0, double forceInitInclination = 0.0);
    bool connected() override;
    double minStepInclination() override;
    double minStepSpeed();
    bool autoPauseWhenSpeedIsZero();
    bool autoStartWhenSpeedIsGreaterThenZero();

  private:
    double GetSpeedFromPacket(const QByteArray &packet, int index);
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    bool noConsole = false;
    bool noHeartService = false;
    uint32_t pollDeviceTime = 200;
    uint8_t sec1Update = 0;
    uint8_t firstInit = 0;
    QByteArray lastPacket;
    QDateTime lastTimeCharacteristicChanged;
    bool firstCharacteristicChanged = true;

    QByteArray actualPaceSign;
    QByteArray actualPace2Sign;
    QByteArray actualHR;
    QByteArray actualResistance;
    QByteArray actualOdometer;
    int distance = 0;
    QDateTime lastTimeDistance = QDateTime::currentDateTime();
    metric speed;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    bool initDone = false;
    bool initRequest = false;

  Q_SIGNALS:
    void disconnected();
    void debug(QString string);
    void speedChanged(double speed);
    void packetReceived();

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);

    void changeInclinationRequested(double grade, double percentage);
};

#endif // OCTANEELLIPTICAL_H
