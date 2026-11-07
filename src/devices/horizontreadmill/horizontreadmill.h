#ifndef HORIZONTREADMILL_H
#define HORIZONTREADMILL_H

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
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QDateTime>
#include <QObject>
#include <QString>

#include "treadmill.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class horizontreadmill : public treadmill {
    Q_OBJECT
  public:
    horizontreadmill(bool noWriteResistance, bool noHeartService);
    bool connected() override;
    static bool urtmNativeName(const QString &name) { return name.startsWith(QStringLiteral("URTM"), Qt::CaseInsensitive); }
    static bool urtmNativeCapabilities(const QString &name, bool service, bool notify, bool cccd, bool write) {
        return urtmNativeName(name) && service && notify && cccd && write;
    }
    enum class UrtmChecksum { Unknown, ExcludeStx, IncludeStx };
    static bool urtmReady(bool service, bool subscribed, UrtmChecksum checksum) {
        return service && subscribed && checksum != UrtmChecksum::Unknown;
    }
    static bool urtmFresh(bool paired, bool stopping, qint64 statusTime, qint64 now) {
        return paired && !stopping && statusTime > 0 && now >= statusTime && now - statusTime < 5000;
    }
    static bool urtmFallbackSafe(bool paired, bool stopping, qint64 statusTime) {
        return !stopping && (paired || !statusTime);
    }
    static QByteArray urtmCommand(QByteArray frame, UrtmChecksum checksum) {
        if (checksum == UrtmChecksum::Unknown) return {};
        unsigned sum = checksum == UrtmChecksum::IncludeStx ? 2 : 0;
        for (int i = 1; i < frame.size() - 2; ++i) sum += quint8(frame[i]);
        frame[frame.size() - 2] = char((sum & 0xff) ^ 0x5a);
        return frame;
    }
    static QByteArray urtmStart(UrtmChecksum checksum = UrtmChecksum::ExcludeStx) {
        return urtmCommand(QByteArray::fromHex("02530100000000000000000e03"), checksum);
    }
    static QByteArray urtmStop(bool pause, UrtmChecksum checksum = UrtmChecksum::ExcludeStx) {
        return urtmCommand(QByteArray::fromHex(pause ? "02530a0703" : "0253030c03"), checksum);
    }
    static QByteArray urtmTarget(double speed, double incline, UrtmChecksum checksum) {
        if (checksum == UrtmChecksum::Unknown || !qIsFinite(speed) || !qIsFinite(incline) ||
            speed < 0 || speed > 255 * 0.1609344 || incline < 0 || incline > 25.5)
            return {};
        const int s = qRound(speed / 0.1609344);
        const int i = qRound(incline * 10);
        QByteArray frame = QByteArray::fromHex("02530200000003");
        frame[3] = char(s);
        frame[4] = char(i);
        const int stx = checksum == UrtmChecksum::IncludeStx ? 2 : 0;
        frame[5] = char(((stx + 0x53 + 0x02 + s + i) & 0xff) ^ 0x5a);
        return frame;
    }
    // Only the captured 25-byte layout has a proven paired incline field.
    static bool urtmStatus(const QByteArray &frame, double &speed, double &incline, UrtmChecksum &checksum) {
        if (frame == QByteArray::fromHex("025100020903")) {
            if (checksum != UrtmChecksum::Unknown && checksum != UrtmChecksum::ExcludeStx) return false;
            checksum = UrtmChecksum::ExcludeStx;
            return true; // Idle contains no speed/incline pair.
        }
        if (frame.size() != 25 || quint8(frame[0]) != 0x02 || quint8(frame[1]) != 0x51 ||
            quint8(frame[24]) != 0x03)
            return false;
        const quint8 kind = quint8(frame[2]);
        if (kind != 0x01 && kind != 0x03 && kind != 0x04 && kind != 0x0a)
            return false;
        unsigned sum = 0;
        for (int i = 1; i < 23; ++i) sum += quint8(frame[i]);
        const auto found = quint8(frame[23]);
        const auto variant = found == quint8((sum & 0xff) ^ 0x5a) ? UrtmChecksum::ExcludeStx :
                             found == quint8(((sum + 2) & 0xff) ^ 0x5a) ? UrtmChecksum::IncludeStx :
                             UrtmChecksum::Unknown;
        if (variant == UrtmChecksum::Unknown || (checksum != UrtmChecksum::Unknown && checksum != variant))
            return false;
        checksum = variant;
        speed = quint8(frame[3]) * 0.1609344;
        incline = quint8(frame[4]) / 10.0;
        return true;
    }
    // One update step, shared with transport-independent orchestration tests.
    struct UrtmControlRefs {
        bool &urtmSubscribed; bool &urtmPaired; bool &urtmStopping;
        bool &urtmStartUnconfirmed; bool &urtmIdleStartEligible;
        double &urtmSpeed; double &urtmIncline;
        qint64 &urtmStatusTime; qint64 &urtmPendingSince; UrtmChecksum &urtmChecksum;
        volatile double &requestSpeed; double &requestInclination;
        int8_t &requestStart; int8_t &requestStop; int8_t &requestPause;
    };
    static void urtmReset(UrtmControlRefs state, bool hadNativeService) {
        if (hadNativeService) {
            state.requestSpeed = -1; state.requestInclination = -100;
            state.requestStart = state.requestStop = state.requestPause = -1;
        }
        state.urtmSubscribed = state.urtmPaired = state.urtmStopping = state.urtmStartUnconfirmed = false;
        state.urtmIdleStartEligible = false;
        state.urtmStatusTime = state.urtmPendingSince = 0;
        state.urtmChecksum = UrtmChecksum::Unknown;
    }
    enum class UrtmStatusResult { Invalid, Idle, Full };
    static UrtmStatusResult urtmApplyStatus(UrtmControlRefs state, const QByteArray &value, qint64 now) {
        double speed, incline;
        if (!urtmStatus(value, speed, incline, state.urtmChecksum)) return UrtmStatusResult::Invalid;
        if (value.size() == 6) {
            // Idle has no speed and may predate stop; it cannot release the stopping gate.
            if (!state.urtmStartUnconfirmed && !state.urtmStopping &&
                (!state.urtmStatusTime || state.urtmIdleStartEligible)) {
                state.urtmIdleStartEligible = true;
                state.urtmStatusTime = now;
            }
            return UrtmStatusResult::Idle;
        }
        state.urtmSpeed = speed;
        state.urtmIncline = incline;
        state.urtmStatusTime = now;
        if (state.urtmStopping && speed == 0) {
            state.urtmStopping = false;
            state.urtmStartUnconfirmed = false;
        } else if (!state.urtmStopping && speed > 0) {
            state.urtmStartUnconfirmed = false;
        }
        state.urtmPaired = !state.urtmStopping && !state.urtmStartUnconfirmed;
        state.urtmIdleStartEligible = false;
        return UrtmStatusResult::Full;
    }
    template <typename Send, typename Log, typename Started, typename ReverseIncline>
    static bool urtmControlStep(UrtmControlRefs state, bool nativeService, bool ftmsAvailable, qint64 now,
                                Send send, Log log, Started started, ReverseIncline reverseIncline) {
        auto &urtmSubscribed = state.urtmSubscribed;
        auto &urtmPaired = state.urtmPaired;
        auto &urtmStopping = state.urtmStopping;
        auto &urtmStartUnconfirmed = state.urtmStartUnconfirmed;
        auto &urtmIdleStartEligible = state.urtmIdleStartEligible;
        auto &urtmSpeed = state.urtmSpeed;
        auto &urtmIncline = state.urtmIncline;
        auto &urtmStatusTime = state.urtmStatusTime;
        auto &urtmPendingSince = state.urtmPendingSince;
        auto &urtmChecksum = state.urtmChecksum;
        auto &requestSpeed = state.requestSpeed;
        auto &requestInclination = state.requestInclination;
        auto &requestStart = state.requestStart;
        auto &requestStop = state.requestStop;
        auto &requestPause = state.requestPause;
        // A complete GATT shape is not enough: use FTMS until native status
        // establishes the checksum variant and the notification is subscribed.
        const qint64 urtmNow = now;
        const bool urtmStatusFresh = urtmFresh(true, false, urtmStatusTime, urtmNow);
        if (nativeService && requestStop != -1) {
            // Legacy FTMS handles Start before Stop: discard conflicting requests first.
            requestStart = -1;
            requestSpeed = -1;
            requestInclination = -100;
        }
        if (urtmReady(nativeService, urtmSubscribed, urtmChecksum)) {
            if (requestStop != -1) {
                send(urtmStop(requestPause != -1, urtmChecksum));
                urtmStopping = true;
                urtmPaired = false;
                urtmIdleStartEligible = false;
                urtmPendingSince = 0;
                requestStop = requestPause = requestStart = -1;
                requestSpeed = -1;
                requestInclination = -100;
                return true;
            }
            if (urtmStopping) {
                if (requestStart != -1 || requestSpeed != -1 || requestInclination != -100) {
                    log(QStringLiteral("URTM start/target rejected while stopping"));
                    requestStart = requestSpeed = -1;
                    requestInclination = -100;
                }
                return true;
            }
            if (requestStart != -1) {
                if (!urtmStatusFresh || urtmStartUnconfirmed ||
                    (!urtmPaired && !urtmIdleStartEligible) || (urtmPaired && urtmSpeed > 0)) {
                    log(QStringLiteral("URTM start rejected while a native command is unconfirmed"));
                    requestStart = -1;
                    return true;
                }
                send(urtmStart(urtmChecksum));
                requestStart = -1;
                urtmStartUnconfirmed = true;
                urtmIdleStartEligible = false;
                urtmPaired = false; // Wait for new status before sending a combined target.
                started();
                return true;
            }
            if (requestSpeed != -1 || requestInclination != -100) {
                if (!urtmPendingSince) urtmPendingSince = now;
                if (now < urtmPendingSince || now - urtmPendingSince >= 5000) {
                    log(QStringLiteral("URTM target expired waiting for fresh status"));
                    requestSpeed = -1;
                    requestInclination = -100;
                    urtmPendingSince = 0;
                    return true;
                }
                if (!urtmFresh(urtmPaired, urtmStopping, urtmStatusTime, now)) {
                    log(QStringLiteral("URTM target waiting for fresh paired status"));
                    return true; // Do not discard requests while awaiting a safe counterpart.
                }
                const double speed = requestSpeed == -1 ? urtmSpeed : double(requestSpeed);
                const double incline = requestInclination == -100 ? urtmIncline :
                    double(reverseIncline(requestInclination));
                const QByteArray frame = urtmTarget(speed, incline, urtmChecksum);
                if (speed > 22 || incline > 15 || frame.isEmpty()) {
                    log(QStringLiteral("URTM target rejected: out of range or invalid"));
                } else {
                    send(frame);
                    urtmPaired = false; // Require a new status for the next combined command.
                }
                requestSpeed = -1;
                requestInclination = -100;
                urtmPendingSince = 0;
            } else {
                urtmPendingSince = 0;
            }
            return true;
        }
        // A stop takes priority over uncertain native state: try FTMS once when
        // available. Neither the write nor subsequent status proves physical stop.
        if (nativeService && requestStop != -1 && ftmsAvailable) {
            log(QStringLiteral("URTM native unavailable; attempting FTMS stop (physical outcome unknown)"));
            urtmStopping = true;
            urtmPaired = false;
            urtmIdleStartEligible = false;
            urtmPendingSince = 0;
            return false;
        }
        // Do not switch transports while a native command awaits a confirming status.
        if (nativeService && (urtmStartUnconfirmed || (urtmStatusTime && !urtmStatusFresh) ||
                            (!urtmIdleStartEligible && !urtmFallbackSafe(urtmPaired, urtmStopping, urtmStatusTime)) ||
                            !ftmsAvailable)) {
            if (requestStop != -1) {
                log(QStringLiteral("URTM stop rejected: native unavailable or command unconfirmed; FTMS unavailable or unsafe"));
                requestStop = requestPause = -1;
            }
            if (requestStart != -1 || requestSpeed != -1 || requestInclination != -100) {
                log(QStringLiteral("URTM start/target rejected: native not ready and FTMS unavailable"));
                requestStart = requestSpeed = -1;
                requestInclination = -100;
            }
            return true;
        }

        return false; // Continue through the existing FTMS/legacy path.
    }
    void forceSpeed(double requestSpeed);
    void forceIncline(double requestIncline);
    double minStepInclination() override;
    double minStepSpeed() override;

    bool autoPauseWhenSpeedIsZero() override;
    bool autoStartWhenSpeedIsGreaterThenZero() override;

  private:
    void writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic, uint8_t *data,
                             uint8_t data_len, QString info, bool disable_log = false, bool wait_for_response = false);
    void waitForAPacket();
    void startDiscover();
    void btinit();
    void setDirectDistance(double treadmillDistance, bool increment);

    QTimer *refresh;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattWriteCharControlPointId;
    QLowEnergyCharacteristic gattWriteCharControlPointIdYpooMiniPro;
    QLowEnergyService *gattFTMSService = nullptr;
    QLowEnergyService *urtmService = nullptr;
    QLowEnergyCharacteristic urtmWrite;
    QLowEnergyDescriptor urtmNotifyDescriptor;
    bool urtmSubscribed = false;
    bool urtmPaired = false;
    bool urtmStopping = false;
    bool urtmStartUnconfirmed = false;
    bool urtmIdleStartEligible = false;
    double urtmSpeed = 0, urtmIncline = 0;
    qint64 urtmStatusTime = 0;
    qint64 urtmPendingSince = 0;
    UrtmChecksum urtmChecksum = UrtmChecksum::Unknown;
    QLowEnergyCharacteristic gattWriteCharCustomService;
    QLowEnergyService *gattCustomService = nullptr;
    QLowEnergyCharacteristic gattWriteCharMerachUnlock;
    QLowEnergyService *gattMerachUnlockService = nullptr;
    volatile int notificationSubscribed = 0;

    static inline const QBluetoothUuid DomyosServiceId{QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455")};

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QByteArray lastPacketComplete;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    bool firstDistanceCalculated = false;
    double lastDirectDistance = -1;
    QDateTime lastDirectDistanceChanged;
    uint8_t firstStateChanged = 0;
    double lastSpeed = 0.0;
    double lastInclination = 0;
    qint64 lastNonZeroSpeedTimestamp = 0;
    bool horizonPaused = false;
    double lastHorizonForceSpeed = 0;
    double minInclination = 0.0;
    double maxInclination = 15.0;

    bool initDone = false;
    bool initRequest = false;
    bool initPacketRecv = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    int32_t customRecv = 0;
    int32_t messageID = 0;

    bool mobvoi_treadmill = false;
    bool mobvoi_tmp_treadmill = false;
    bool kettler_treadmill = false;
    bool wellfit_treadmill = false;
    bool sole_tt8_treadmill = false;
    bool sole_s77_treadmill = false;
    bool anplus_treadmill = false;
    bool run_bt_treadmill = false;
    bool tunturi_t60_treadmill = false;
    bool trx3500_treadmill = false;
    bool sole_f85_treadmill = false;
    bool sole_f89_treadmill = false;
    bool adidas_treadmill = false;
    bool schwinn_810_treadmill = false;
    bool yesoul_treadmill = false;
    bool technogymrun = false;
    bool disableAutoPause = false;
    bool HORIZON_78AT_treadmill = false;
    bool ICONCEPT_FTMS_treadmill = false;
    bool iconcept_ftms_treadmill_inclination_table = false;
    bool DOMYOS = false;
    bool domyos_treadmill_ts100 = false;
    bool SW_TREADMILL = false;
    bool SW3925EAI_TREADMILL = false;
    bool BOWFLEX_T9 = false;
    bool YPOO_MINI_PRO = false;
    bool MX_TM = false;
    bool FIT = false;
    bool FIT_TM = false;
    bool T3G_PRO = false;
    bool T3G_ELITE = false;
    bool JFTM_T202 = false;
    bool TP1 = false;
    bool T01 = false;
    bool TM4800 = false;
    bool TM4500 = false;
    bool TM6500 = false;
    bool TM55_TREADMILL = false;
    bool FS_TREADMILL = false;
    bool SF_TREADMILL = false;
    bool WT_TREADMILL = false;
    bool THERUN_T15 = false;
    bool MERACH_TREADMILL = false;

    void testProfileCRC();
    void updateProfileCRC();
    int GenerateCRC_CCITT(uint8_t *PUPtr8, int PU16_Count, int crcStart = 65535);
    bool checkIfForceSpeedNeeding(double requestSpeed);
    float float_one_point_round(float value);

    // profiles
    uint8_t initData7[20] = {0x55, 0xaa, 0x02, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xed, 0xc2,
                             0x00, 0x47, 0x75, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData8[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x05, 0xc2, 0x07};
    uint8_t initData10[20] = {0x01, 0x01, 0x00, 0xd3, 0x8a, 0x0c, 0x00, 0x01, 0x01, 0x02,
                              0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t initData11[20] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                              0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t initData12[20] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                              0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData13[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30,
                              0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t initData14[1] = {0x30};

    uint8_t initData7_1[20] = {0x55, 0xaa, 0x03, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xae, 0x2a,
                               0x01, 0x41, 0x69, 0x61, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_1[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x06, 0xc4, 0x07};
    uint8_t initData10_1[20] = {0x09, 0x1c, 0x00, 0x9f, 0xef, 0x0c, 0x00, 0x01, 0x01, 0x02,
                                0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_2[20] = {0x55, 0xaa, 0x04, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xae, 0x2a,
                               0x01, 0x41, 0x69, 0x61, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_2[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x06, 0xc4, 0x07};
    uint8_t initData10_2[20] = {0x09, 0x1c, 0x00, 0x9f, 0xef, 0x0c, 0x00, 0x01, 0x01, 0x02,
                                0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_3[20] = {0x55, 0xaa, 0x05, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xa9, 0xe7,
                               0x02, 0x4d, 0x65, 0x67, 0x68, 0x61, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_3[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x06, 0xc5, 0x07};
    uint8_t initData10_3[20] = {0x0b, 0x0f, 0x00, 0x4b, 0x40, 0x0c, 0x00, 0x01, 0x01, 0x02,
                                0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_4[20] = {0x55, 0xaa, 0x06, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xbc, 0x76,
                               0x03, 0x44, 0x61, 0x72, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_4[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x07, 0xca, 0x07};
    uint8_t initData10_4[20] = {0x05, 0x1c, 0x00, 0x07, 0x25, 0x0c, 0x00, 0x01, 0x01, 0x02,
                                0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_5[20] = {0x55, 0xaa, 0x07, 0x00, 0x01, 0x16, 0xdb, 0x02, 0x7d, 0xeb,
                               0x04, 0x41, 0x68, 0x6f, 0x6e, 0x61, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_5[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x04, 0xcc, 0x07};
    uint8_t initData10_5[20] = {0x01, 0x08, 0x00, 0xc2, 0x0f, 0x0c, 0x00, 0x01, 0x01, 0x02,
                                0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_6[20] = {0x55, 0xaa, 0x08, 0x00, 0x01, 0x16, 0xdb, 0x02, 0x03, 0x0d,
                               0x05, 0x55, 0x73, 0x65, 0x72, 0x20, 0x35, 0x00, 0x00, 0x00};
    uint8_t initData9_6[20] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x05, 0xc2, 0x07};
    uint8_t initData10_6[20] = {0x01, 0x01, 0x00, 0x8e, 0x6a, 0x0c, 0x00, 0x01, 0x01, 0x02,
                                0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    // profiles end

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

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);

    void changeInclinationRequested(double grade, double percentage);
    void ftmsCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
};

#endif // HORIZONTREADMILL_H
