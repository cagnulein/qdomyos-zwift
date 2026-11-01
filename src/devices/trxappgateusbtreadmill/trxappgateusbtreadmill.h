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
    struct FlowFitnessMetrics {
        bool valid = false;
        double speed = 0.0;
        double inclination = 0.0;
        double kcal = 0.0;
        double distance = 0.0;
        uint16_t heart = 0;
        uint16_t elapsed = 0;
    };

    class IconsoleTransactionScope {
      public:
        IconsoleTransactionScope(bool &state, bool protectTransaction)
            : transactionInProgress(state), previousState(state) {
            if (protectTransaction) {
                transactionInProgress = true;
            }
        }
        ~IconsoleTransactionScope() { transactionInProgress = previousState; }
        IconsoleTransactionScope(const IconsoleTransactionScope &) = delete;
        IconsoleTransactionScope &operator=(const IconsoleTransactionScope &) = delete;

      private:
        bool &transactionInProgress;
        bool previousState;
    };

    static bool shouldSkipIconsoleTransaction(bool isIconsole, bool transactionInProgress) {
        return isIconsole && transactionInProgress;
    }

    trxappgateusbtreadmill();
    bool connected() override;

    double minStepInclination() override;
    bool canHandleSpeedChange() override { return false; }

    static QByteArray flowFitnessStartPacket();
    static QByteArray flowFitnessStopPacket();
    static FlowFitnessMetrics flowFitnessMetricsFromPacket(const QByteArray &packet);
    static bool isFlowFitnessDeviceName(const QString &deviceName);
    static bool isIconsoleTx90DeviceName(const QString &deviceName);
    static QByteArray iconsoleSpeedProgramFrame(quint8 command, const QList<double> &speeds);
    static QByteArray iconsoleKeepAliveFrame();
    static bool shouldAutoStartFromTelemetry(bool isIconsoleTreadmill, bool readyStatusPattern, bool alreadyReady);
    static bool shouldRetryIconsoleInitialization(int completedAttempts);
    static bool shouldAcceptIconsoleUploadCompletion(bool uploadSucceeded, bool latestRequestInvalidated);
    static bool shouldBlockIconsoleStart(bool uploadRequired, bool uploadPending, bool uploadSucceeded,
                                         bool latestRequestInvalidated);
    static bool shouldContinueIconsoleUpload(bool frameWriteSucceeded, quint64 activeGeneration,
                                             quint64 currentGeneration);
    static bool hasNewerPendingIconsoleWorkout(bool hasPendingFrames, quint64 activeGeneration,
                                               quint64 pendingGeneration);
    static QList<QByteArray> iconsoleInitializationFrames();
    static int iconsoleExpectedResponseSize(quint8 requestCommand);
    enum class IconsoleRunState { Unknown, Ready, Running, Paused };
    static IconsoleRunState iconsoleRunStateFromPacket(const QByteArray &packet);
    static bool takeIconsoleResponse(QByteArray &buffer, QByteArray &frame);
    static bool iconsoleResponseMatches(quint8 requestCommand, const QByteArray &response);
    static bool buildIconsoleWorkoutFrames(const QList<trainrow> &rows, QList<QByteArray> *frames,
                                           QString *error = nullptr);

    void setTrainingProgram(const QList<trainrow> &rows) override;
    bool isIconsoleTreadmill() const { return iconsoleTreadmill; }

  private:
    static double GetSpeedFromPacket(const QByteArray &packet);
    static double GetInclinationFromPacket(const QByteArray &packet);
    static double GetKcalFromPacket(const QByteArray &packet);
    static double GetDistanceFromPacket(const QByteArray &packet);
    static uint16_t GetHeartFromPacket(const QByteArray &packet);
    static uint16_t GetElapsedFromPacket(const QByteArray &packet);
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    void uploadPendingIconsoleWorkout();
    void processTelemetryPacket();
    void updateDisplay(uint16_t elapsed);
    void btinit(bool startTape);
    bool writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                             bool wait_for_response, quint8 expectedIconsoleResponse = 0);
    void waitForAPacket();
    void startDiscover();
    QTimer *refresh;

    uint8_t firstVirtualTreadmill = 0;
    bool firstCharChanged = true;
    QTime lastTimeCharChanged;
    uint8_t sec1update = 0;
    QByteArray lastPacket;
    QByteArray iconsoleReceiveBuffer;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotifyCharacteristic;
    QLowEnergyCharacteristic gattNotify2Characteristic;

    bool initDone = false;
    bool initRequest = false;
    int iconsoleInitializationAttempts = 0;
    bool readyToStart = false;
    bool iconsoleTreadmill = false;
    bool iconsoleTransactionInProgress = false;
    bool iconsoleAwaitingBeltStart = false;
    bool iconsoleStartWaitLogged = false;
    bool iconsoleWorkoutUploadRequired = false;
    bool iconsoleWorkoutUploadPending = false;
    bool iconsoleWorkoutUploadSucceeded = false;
    bool iconsoleWorkoutRequestInvalidated = false;
    quint64 iconsoleWorkoutRequestGeneration = 0;
    quint64 pendingIconsoleWorkoutGeneration = 0;
    QList<QByteArray> pendingIconsoleWorkoutFrames;

    typedef enum TYPE { TRXAPPGATE = 0, IRUNNING = 1, REEBOK = 2, DKN = 3, DKN_2 = 4, IRUNNING_2 = 5, ADIDAS = 6, REEBOK_2 = 7, DKN_3 = 8, FLOW_FITNESS = 9 } TYPE;
    TYPE treadmill_type = TRXAPPGATE;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived(const QByteArray &packet);

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
