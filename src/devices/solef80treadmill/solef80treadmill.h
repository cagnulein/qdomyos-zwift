#ifndef SOLEF80_H
#define SOLEF80_H

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
//#include <QtBluetooth/private/qlowenergycontrollerbase_p.h>
//#include <QtBluetooth/private/qlowenergyserviceprivate_p.h>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>
#include <QString>

#include "treadmill.h"

// Gate motion commands on fresh vendor telemetry, not just ATT acknowledgements.
class SoleF63TelemetryGuard {
  public:
    bool recordVendorPacket(const QByteArray &packet, qint64 nowMs) {
        if (packet.size() != 18 || packet.at(0) != char(0x5b) || packet.at(1) != char(0x0f) ||
            packet.at(2) != char(0x06) || packet.at(17) != char(0x5d))
            return false;
        lastTelemetryMs = nowMs;
        return true;
    }
    bool isFresh(qint64 nowMs) const {
        return lastTelemetryMs >= 0 && nowMs >= lastTelemetryMs && nowMs - lastTelemetryMs <= 5000;
    }
    void reset() { lastTelemetryMs = -1; }

  private:
    qint64 lastTelemetryMs = -1;
};

class SoleF63InitRetryState {
  public:
    void begin() {
        waitingForResponse = true;
        responseReady = false;
        attempts = 0;
    }

    bool beginAfterNotifyCccdWrite(bool f63Model, bool currentNotifyCccd, bool notificationsEnabled) {
        if (!f63Model || !currentNotifyCccd || !notificationsEnabled)
            return false;

        begin();
        return true;
    }

    bool shouldRetry() const { return waitingForResponse; }

    void recordAttempt() {
        if (waitingForResponse)
            ++attempts;
    }

    bool receiveNotification(const QByteArray &packet) {
        // Only the BM70 F0 reply can complete the initial handshake.
        if (!waitingForResponse || attempts == 0 || packet != QByteArray::fromHex("5b08f0900000b40a0f125d"))
            return false;

        waitingForResponse = false;
        responseReady = true;
        return true;
    }

    bool takeResponse() {
        const bool ready = responseReady;
        responseReady = false;
        return ready;
    }

    void cancel() {
        waitingForResponse = false;
        responseReady = false;
        attempts = 0;
    }

    quint64 attemptCount() const { return attempts; }

  private:
    bool waitingForResponse = false;
    bool responseReady = false;
    quint64 attempts = 0;
};

// Only frames actually observed in Olivier's October 19 SOLE Fitness capture.
class SoleF63NativeProtocol {
  public:
    static QList<QByteArray> startFrames() {
        return {
            QByteArray::fromHex("5b06070123009baa5d"),
            QByteArray::fromHex("5b030820025d"),
            QByteArray::fromHex("5b0504000000005d"),
            QByteArray::fromHex("5b0222095d"),
            QByteArray::fromHex("5b0222095d"),
            QByteArray::fromHex("5b0202025d"),
            QByteArray::fromHex("5b0400104f4b5d"),
            QByteArray::fromHex("5b0400104f4b5d"),
            QByteArray::fromHex("5b0400404f4b5d"),
            QByteArray::fromHex("5b0400404f4b5d"),
            QByteArray::fromHex("5b0400104f4b5d"),
            QByteArray::fromHex("5b0400104f4b5d"),
            QByteArray::fromHex("5b0203035d"),
            QByteArray::fromHex("5b0203035d"),
            QByteArray::fromHex("5b0203035d")
        };
    }
    static QList<QByteArray> stopFrames() {
        return {
            QByteArray::fromHex("5b0203065d"),
            QByteArray::fromHex("5b0203065d"),
            QByteArray::fromHex("5b0203065d")
        };
    }
};

// Mirrors the proven passive F63 bridge: respond only to incoming vendor frames.
// No unsolicited mode/Start/Stop/motion commands are ever generated here.
class SoleF63PassiveReplies {
  public:
    void observe(const QByteArray &frame, qint64 nowMs) {
        if (frame.size() < 5 || quint8(frame.at(0)) != 0x5b ||
            frame.size() != quint8(frame.at(1)) + 3 || quint8(frame.at(frame.size() - 1)) != 0x5d)
            return;
        const quint8 type = quint8(frame.at(2));
        if (type == 0x03 && frame.size() == 5) {
            // A later mode supersedes the previous state echo.
            pending.remove(type);
            const quint8 mode = quint8(frame.at(3));
            if (mode == 1 || mode == 3 || mode == 4 || mode == 6 || mode == 7 || mode == 0x80)
                pending.insert(type, Pending{frame, nowMs});
        } else if ((type == 0x06 && frame.size() == 18) ||
                   (type == 0x09 && frame.size() == 6) ||
                   (type == 0x10 && frame.size() == 5) ||
                   (type == 0x32 && frame.size() == 13) ||
                   (type == 0x40 && frame.size() == 22)) {
            QByteArray ack = QByteArray::fromHex("5b0400004f4b5d");
            ack[3] = char(type);
            pending.insert(type, Pending{ack, nowMs}); // coalesce duplicate type
        }
        // Never acknowledge acknowledgements, F0 identity or unknown frames.
    }
    QByteArray nextReply(qint64 nowMs) {
        while (!pending.isEmpty()) {
            auto it = pending.begin();
            const Pending reply = it.value();
            pending.erase(it);
            if (nowMs >= reply.timestamp && nowMs - reply.timestamp <= 900)
                return reply.frame;
        }
        return QByteArray();
    }
    int pendingCount() const { return pending.size(); }
    void reset() { pending.clear(); }

  private:
    struct Pending { QByteArray frame; qint64 timestamp; };
    QMap<quint8, Pending> pending;
};

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class solef80treadmill : public treadmill {
    Q_OBJECT
  public:
    solef80treadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    void stopWhenIdle() override;
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    double minStepInclination() override;

  private:
    void writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log = false,
                             bool wait_for_response = false);
    void waitForAPacket();
    void startDiscover();
    void btinit();
    void sendF63Stop();
    void writeF63Frame(const QByteArray &frame, const QString &label);
    void queueF63Start();
    void sendF63SequenceStep();
    void f63PollTick();
    void resetF63Session();

    QTimer *refresh;
    QTimer f63InitRetryTimer;
    SoleF63InitRetryState f63InitRetryState;
    SoleF63TelemetryGuard f63TelemetryGuard;
    enum class F63Phase { Handshake, Ready, Preparing, AwaitingTelemetry, Running, Stopping };
    F63Phase f63Phase = F63Phase::Handshake;
    QTimer f63CommandTimer;
    QTimer f63PollTimer;
    SoleF63PassiveReplies f63Replies;
    qint64 f63LastWriteMs = -1;
    bool f63RxSeen = false;
    QList<QByteArray> f63Frames;
    int f63FrameIndex = 0;
    int f63PollCounter = 0;
    qint64 f63StartDeadlineMs = -1;
    qint64 f63LastMotionMs = -1;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyService *gattFTMSService = nullptr;
    QLowEnergyCharacteristic gattWriteCharCustomService;
    QLowEnergyService *gattCustomService = nullptr;
    QLowEnergyDescriptor f63NotifyCccdDescriptor;
    bool f63NotifyCccdWritePending = false;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    double lastSpeed = 0.0;
    double lastInclination = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    enum _REQUEST_STATE { IDLE = 0, UP = 1, DOWN = 2 };
    _REQUEST_STATE requestSpeedState = IDLE;
    _REQUEST_STATE requestInclinationState = IDLE;

    typedef enum TYPE {
        F80 = 0,
        F63 = 1,
        TRX7_5 = 2,
    } TYPE;
    volatile TYPE treadmill_type = F80;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

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
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void changeInclinationRequested(double grade, double percentage);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void retryF63InitHandshake();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // SOLEF80_H
