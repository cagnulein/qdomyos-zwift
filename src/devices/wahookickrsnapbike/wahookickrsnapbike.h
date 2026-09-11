#ifndef WAHOOKICKRSNAPBIKE_H
#define WAHOOKICKRSNAPBIKE_H

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
#include <QtCore/qelapsedtimer.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>
#include <QtCore/qqueue.h>

#include <QDateTime>
#include <QObject>
#include <QString>

#include "wheelcircumference.h"
#include "devices/bike.h"
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class wahookickrsnapbike : public bike {
    Q_OBJECT
  public:
    wahookickrsnapbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                       double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    bool connected() override;
    resistance_t maxResistance() override { return 100; }
    bool inclinationAvailableByHardware() override;
    bool inclinationAvailableBySoftware() override { return true; }
    double maxGears() override;
    double minGears() override;


    enum OperationCode : uint8_t {
        _unlock = 32,
        _setResistanceMode = 64,
        _setStandardMode = 65,
        _setErgMode = 66,
        _setSimMode = 67,
        _setSimCRR = 68,
        _setSimWindResistance = 69,
        _setSimGrade = 70,
        _setSimWindSpeed = 71,
        _setWheelCircumference = 72,
    };

    enum CommandGroup : uint8_t {
        _noCommandGroup = 0,
        _inclinationCommandGroup = 1,
        _gearCommandGroup = 2,
    };
    
    // Variabili per iOS (pubbliche per permettere all'implementazione iOS di impostarle)
    bool zwift_found = false;
    bool wahoo_found = false;
        
    // Wrapper per characteristicChanged che accetta direttamente QBluetoothUuid
    void handleCharacteristicValueChanged(const QBluetoothUuid &uuid, const QByteArray &newValue);

  private:
    // Structure for async write queue
    struct WriteRequest {
        QByteArray data;
        QString info;
        bool disable_log;
        bool wait_for_response;
        uint8_t response_opcode = 0;
        uint8_t coalesce_key = _noCommandGroup;
    };
    QByteArray unlockCommand();
    QByteArray setResistanceMode(double resistance);
    QByteArray setStandardMode(uint8_t level);
    QByteArray setErgMode(uint16_t watts);
    QByteArray setSimMode(double weight, double rollingResistanceCoefficient, double windResistanceCoefficient);
    QByteArray setSimCRR(double rollingResistanceCoefficient);
    QByteArray setSimWindResistance(double windResistanceCoefficient);
    QByteArray setSimGrade(double grade);
    QByteArray setSimWindSpeed(double metersPerSecond);
    QByteArray setWheelCircumference(double millimeters);

    bool writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log = false,
                             bool wait_for_response = false, uint8_t response_opcode = 0,
                             uint8_t coalesce_key = _noCommandGroup);
    void processWriteQueue();
    bool isExpectedCommandResponse(const QBluetoothUuid &uuid, const QByteArray &value) const;
    void logCommandResponse(const QByteArray &value);
    uint16_t wattsFromResistance(double resistance);
    metric ResistanceFromFTMSAccessory;
    void startDiscover();
    uint16_t watts() override;

    QTimer *refresh;
    virtualbike *virtualBike = nullptr;

    // Bluetooth LE services and characteristics (unified for all platforms)
    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyService *gattPowerChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;

    // Async write queue management
    QQueue<WriteRequest> writeQueue;
    bool isWriting = false;
    bool currentWriteWaitingForResponse = false;
    uint8_t currentWriteResponseOpcode = 0;
    QString currentWriteInfo;
    QElapsedTimer currentWriteTimer;
    quint64 commandAckCount = 0;
    quint64 commandAckTotalMs = 0;
    QTimer *writeTimeoutTimer = nullptr;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    double lastGearValue = -1;
    double lastGearModifierValue = 0;
    double lastGrade = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;

    bool WAHOO_KICKR = false;
    bool KICKR_BIKE = false;
    bool KICKR_SNAP = false;
    
    bool lastCommandErgMode = false;

    volatile int notificationSubscribed = 0;

    resistance_t lastForcedResistance = -1;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void resistanceFromFTMSAccessory(resistance_t res) override;
    void restoreDefaultWheelDiameter();
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

  private slots:
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
    void inclinationChanged(double grade, double percentage);
};
#endif // WAHOOKICKRSNAPBIKE_H
