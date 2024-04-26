#ifndef BOWFLEXT216TREADMILL_H
#define BOWFLEXT216TREADMILL_H

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

#include <QThread>
#include <QSemaphore>

#include <QtSerialBus/QModbusClient>
#include <QtSerialBus/QModbusDataUnit>
#include <QtSerialBus/QModbusReply>
#include <QtSerialBus/QModbusRtuSerialMaster>
#include <QtSerialPort/QSerialPort>

#include "treadmill.h"

class modbusWorkerThread : public QThread
{
    public:
        explicit modbusWorkerThread(QObject *parent, QString name = "", uint8_t pinUp = 0, uint8_t pinDown = 0, double step = 0.0, double currentValue = 0.0, QSemaphore *semaphore = nullptr);
        void run();
        void setRequestValue(double request);
        void setCurrentValue(double current);
    private:
        QString name;
        double requestValue;
        double currentValue;
        uint8_t pinUp;
        uint8_t pinDown;
        double step;
        const uint16_t GPIO_KEEP_MS = 1;
        const uint16_t GPIO_REBOUND_MS = 175;
        QSemaphore *semaphore;
};

class bowflext216treadmill : public treadmill {
    Q_OBJECT
  public:
    bowflext216treadmill(uint32_t poolDeviceTime = 200, bool noConsole = false, bool noHeartService = false,
                         double forceInitSpeed = 0.0, double forceInitInclination = 0.0);
    ~bowflext216treadmill();
    bool connected() override;
    double minStepSpeed() override;
    double minStepInclination() override;
    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;
    bool canHandleSpeedChange() override { return false; }
    bool canHandleInclineChange() override { return false; }

    static void digitalWrite(int pin, int state);
    static QModbusReply *lastRequest;
    static QModbusClient *modbusDevice;

  private:

    const uint8_t OUTPUT_SPEED_UP = 0;
    const uint8_t OUTPUT_SPEED_DOWN = 1;
    const uint8_t OUTPUT_INCLINE_UP = 2;
    const uint8_t OUTPUT_INCLINE_DOWN = 3;
    const uint8_t OUTPUT_START = 4;
    const uint8_t OUTPUT_STOP = 5;

    const uint16_t GPIO_KEEP_MS = 50;
    //const uint16_t GPIO_REBOUND_MS = 200;
    
    const double SPEED_STEP = 1.60934 / 10.0;
    const double INCLINATION_STEP = 1.0;

    modbusWorkerThread* speedThread;
    modbusWorkerThread* inclineThread;
    QSemaphore *semaphore; // my treadmill don't like it if the buttons will be pressed simultanly


    double GetSpeedFromPacket(const QByteArray &packet);
    double GetInclinationFromPacket(const QByteArray &packet);
    double GetKcalFromPacket(const QByteArray &packet);
    double GetDistanceFromPacket(const QByteArray &packet);
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

    int64_t lastStart = 0;
    int64_t lastStop = 0;

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;
    QLowEnergyCharacteristic gattNotify2Characteristic;
    QLowEnergyCharacteristic gattNotify3Characteristic;
    QLowEnergyCharacteristic gattNotify4Characteristic;
    QLowEnergyCharacteristic gattNotify5Characteristic;

    bool initDone = false;
    bool initRequest = false;

    bool bowflex_t6 = false;
    bool bowflex_btx116 = false;
    bool bowflex_t8j = false;

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
};

#endif // BOWFLEXT216TREADMILL_H
