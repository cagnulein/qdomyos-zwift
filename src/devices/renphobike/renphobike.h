#ifndef renphobike_H
#define renphobike_H

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

class renphobike : public bike {
    Q_OBJECT
  public:
    renphobike(bool noWriteResistance, bool noHeartService);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    // uint8_t resistanceFromPowerRequest(uint16_t power);
    bool connected() override;
    resistance_t maxResistance() override { return max_resistance; }

    // Deterministic, gear-independent resistance implied purely by the current ROUVY/Zwift
    // grade. Mirrors CharacteristicWriteProcessor::changeSlope()'s formula, computed locally
    // so it never depends on the optional "Force Resistance" setting (virtualbike_forceresistance)
    // -- renphobike never overrides virtualbike's default bikeResistanceGain/bikeResistanceOffset,
    // so those defaults (1.0 / 4) are used here too.
    static double autoResistanceFromSlope(int16_t iresistance, uint8_t crr, uint8_t cw, double CRRGain,
                                          double CWGain, double bikeResistanceGain = 1.0,
                                          int bikeResistanceOffset = 4);

    // Reconciles QZ's virtual gear with the physical RENPHO resistance knob.
    //
    // Instead of inferring gear steps from raw resistance deltas (which are also driven by
    // ROUVY's simulated grade and by the bike's own speed-dependent physics -- see issue #4873),
    // this tracks an *expected* resistance (auto resistance from grade, plus the current gear
    // offset) and only reconciles the gear when the bike's actual reported resistance disagrees
    // with that expectation in a sustained way. A reading only counts once it has been observed
    // twice in a row with the same expectation in effect, so neither a single noisy BLE sample
    // nor the settling period right after a new grade/gear write can move the gear.
    // Kept as a plain, Qt-BLE-free struct so it can be unit tested in isolation.
    struct ResistanceReconciler {
        bool hasExpectation = false;
        double expectedResistance = 0.0;
        double pendingDiscrepancy = 0.0;
        int pendingCount = 0;

        // Call whenever the expected resistance changes (new grade and/or new gear). Resets
        // the debounce so the bike has time to settle before a fresh discrepancy is trusted.
        void setExpected(double expected);

        // Feeds a new resistance reading from the bike. Returns the confirmed, stable
        // discrepancy (actual - expected) once it has been observed twice in a row against the
        // same expectation, or 0 if there's nothing new/confirmed yet.
        double feed(double actualResistance);
    };

  private:
    const resistance_t max_resistance = 80;
    double bikeResistanceToPeloton(double resistance);
    void writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    uint16_t ergModificator(uint16_t powerRequested);
    uint16_t watts() override;
    void forceResistance(resistance_t requestResistance);
    void forcePower(int16_t requestPower);

    QTimer *refresh;    
    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    QByteArray lastFTMSPacketReceived;
    resistance_t lastRequestResistance = -1;
    double lastPowerRequestedFactor = 1;
    ResistanceReconciler resistanceReconciler;
    double m_autoResistanceBaseline = 0.0; // last autoResistanceFromSlope() result
    resistance_t m_lastWrittenResistance = -1; // last value actually sent via forceResistance()

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

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
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);

    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // renphobike_H
