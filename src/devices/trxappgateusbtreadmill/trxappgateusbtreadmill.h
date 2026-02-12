#ifndef TRXAPPGATEUSBTREADMILL_H
#define TRXAPPGATEUSBTREADMILL_H

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

#include <QObject>
#include <QTime>

#include "treadmill.h"

class trxappgateusbtreadmill : public treadmill {
    Q_OBJECT
  public:
    trxappgateusbtreadmill();
    bool connected() override;

    double minStepInclination() override;
    bool canHandleSpeedChange() override { return false; }

  private:
    double GetSpeedFromPacket(const QByteArray &packet);
    double GetInclinationFromPacket(const QByteArray &packet);
    double GetKcalFromPacket(const QByteArray &packet);
    double GetDistanceFromPacket(const QByteArray &packet);
    uint16_t GetElapsedFromPacket(const QByteArray &packet);
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                             bool wait_for_response);
    void waitForAPacket();
    void startDiscover();
    double DistanceCalculated = 0;

    QTimer *refresh;

    uint8_t firstVirtualTreadmill = 0;
    bool firstCharChanged = true;
    QTime lastTimeCharChanged;
    uint8_t sec1update = 0;
    QByteArray lastPacket;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;
    QLowEnergyCharacteristic gattNotify2Characteristic;

    bool initDone = false;
    bool initRequest = false;
    bool readyToStart = false;

    typedef enum TYPE { TRXAPPGATE = 0, IRUNNING = 1, REEBOK = 2, DKN = 3, DKN_2 = 4, IRUNNING_2 = 5, ADIDAS = 6, REEBOK_2 = 7, DKN_3 = 8 } TYPE;
    TYPE treadmill_type = TRXAPPGATE;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

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

#endif // TRXAPPGATEUSBTREADMILL_H
