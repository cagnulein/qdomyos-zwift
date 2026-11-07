#include "horizontreadmill/horizontreadmill.h"
#include <gtest/gtest.h>
#include <limits>

TEST(Urtm047Protocol, CapturedIdleLearnsChecksumWithoutInventingPair) {
    using H = horizontreadmill;
    auto checksum = H::UrtmChecksum::Unknown;
    double speed = -123, incline = -456;
    EXPECT_TRUE(H::urtmStatus(QByteArray::fromHex("025100020903"), speed, incline, checksum));
    EXPECT_EQ(checksum, H::UrtmChecksum::ExcludeStx);
    EXPECT_DOUBLE_EQ(speed, -123);
    EXPECT_DOUBLE_EQ(incline, -456);
    auto bad = QByteArray::fromHex("025100020803");
    checksum = H::UrtmChecksum::Unknown;
    EXPECT_FALSE(H::urtmStatus(bad, speed, incline, checksum));
    EXPECT_EQ(checksum, H::UrtmChecksum::Unknown);
    checksum = H::UrtmChecksum::IncludeStx;
    EXPECT_FALSE(H::urtmStatus(QByteArray::fromHex("025100020903"), speed, incline, checksum));
    EXPECT_EQ(checksum, H::UrtmChecksum::IncludeStx);
}

TEST(Urtm047Protocol, ReadinessFreshnessAndSafety) {
    using H = horizontreadmill;
    using C = H::UrtmChecksum;
    EXPECT_FALSE(H::urtmReady(false, true, C::ExcludeStx));
    EXPECT_FALSE(H::urtmReady(true, false, C::ExcludeStx));
    EXPECT_FALSE(H::urtmReady(true, true, C::Unknown));
    EXPECT_TRUE(H::urtmReady(true, true, C::IncludeStx));
    EXPECT_TRUE(H::urtmFresh(true, false, 1000, 5999));
    EXPECT_FALSE(H::urtmFresh(true, false, 1000, 6000));
    EXPECT_FALSE(H::urtmFresh(true, false, 1000, 999));
    EXPECT_FALSE(H::urtmFresh(false, false, 1000, 1100));
    EXPECT_FALSE(H::urtmFresh(true, true, 1000, 1100));
    EXPECT_TRUE(H::urtmFallbackSafe(false, false, 0));
    EXPECT_TRUE(H::urtmFallbackSafe(true, false, 1000));
    EXPECT_FALSE(H::urtmFallbackSafe(false, false, 1000));
    EXPECT_FALSE(H::urtmFallbackSafe(true, true, 1000));
    EXPECT_TRUE(H::urtmTarget(-1, 0, C::ExcludeStx).isEmpty());
    EXPECT_TRUE(H::urtmTarget(1, -1, C::ExcludeStx).isEmpty());
    EXPECT_TRUE(H::urtmTarget(std::numeric_limits<double>::quiet_NaN(), 0, C::ExcludeStx).isEmpty());
    EXPECT_TRUE(H::urtmStart(C::Unknown).isEmpty());
    EXPECT_EQ(H::urtmStart(C::IncludeStx).toHex(), QByteArray("02530100000000000000000c03"));
    EXPECT_EQ(H::urtmStop(false, C::IncludeStx).toHex(), QByteArray("0253030203"));
    EXPECT_EQ(H::urtmStop(true, C::IncludeStx).toHex(), QByteArray("02530a0503"));
}

TEST(Urtm047Protocol, MphConversionAndVariantLock) {
    using H = horizontreadmill;
    auto status = QByteArray::fromHex("0251031428610003002f000100000000000b00000000007503");
    double speed = 0, incline = 0;
    auto checksum = H::UrtmChecksum::Unknown;
    ASSERT_TRUE(H::urtmStatus(status, speed, incline, checksum));
    EXPECT_EQ(checksum, H::UrtmChecksum::ExcludeStx);
    EXPECT_NEAR(speed, 20 * 0.1609344, 1e-9);
    EXPECT_EQ(H::urtmTarget(speed, incline, checksum).toHex(), QByteArray("0253021428cb03"));
    unsigned sum = 0;
    for (int i = 0; i < 23; ++i) sum += quint8(status[i]);
    status[23] = char((sum & 255) ^ 0x5a);
    checksum = H::UrtmChecksum::Unknown;
    ASSERT_TRUE(H::urtmStatus(status, speed, incline, checksum));
    EXPECT_EQ(checksum, H::UrtmChecksum::IncludeStx);
    EXPECT_EQ(H::urtmTarget(speed, incline, checksum).toHex(), QByteArray("0253021428c903"));
    EXPECT_TRUE(H::urtmTarget(speed, incline, H::UrtmChecksum::Unknown).isEmpty());
}

TEST(Urtm047Protocol, TargetRoundsKmhToNearestTenthMph) {
    using H = horizontreadmill;
    using C = H::UrtmChecksum;
    const auto below = H::urtmTarget(20.49 * 0.1609344, 4.0, C::ExcludeStx);
    const auto above = H::urtmTarget(20.51 * 0.1609344, 4.0, C::ExcludeStx);
    ASSERT_EQ(below.size(), 7);
    ASSERT_EQ(above.size(), 7);
    EXPECT_EQ(quint8(below[3]), 20);
    EXPECT_EQ(quint8(above[3]), 21);
    EXPECT_EQ(quint8(below[4]), 40);
    EXPECT_EQ(quint8(above[4]), 40);
    EXPECT_TRUE(H::urtmTarget(std::numeric_limits<double>::infinity(), 0, C::ExcludeStx).isEmpty());
    EXPECT_TRUE(H::urtmTarget(0, std::numeric_limits<double>::infinity(), C::ExcludeStx).isEmpty());
}

TEST(Urtm047Protocol, RejectsVariantChangeWithoutUpdatingPairedValues) {
    using H = horizontreadmill;
    auto status = QByteArray::fromHex("0251031428610003002f000100000000000b00000000007503");
    double speed = -1, incline = -1;
    auto checksum = H::UrtmChecksum::Unknown;
    ASSERT_TRUE(H::urtmStatus(status, speed, incline, checksum));
    const double previousSpeed = speed, previousIncline = incline;
    unsigned sum = 0;
    for (int i = 0; i < 23; ++i) sum += quint8(status[i]);
    status[23] = char((sum & 255) ^ 0x5a);
    EXPECT_FALSE(H::urtmStatus(status, speed, incline, checksum));
    EXPECT_EQ(checksum, H::UrtmChecksum::ExcludeStx);
    EXPECT_DOUBLE_EQ(speed, previousSpeed);
    EXPECT_DOUBLE_EQ(incline, previousIncline);
}

TEST(Urtm047Protocol, CaptureVectorsAndName) {
    EXPECT_TRUE(horizontreadmill::urtmTarget(20 * 0.1609344, 4.0, horizontreadmill::UrtmChecksum::ExcludeStx).toHex() == "0253021428cb03");
    EXPECT_TRUE(horizontreadmill::urtmTarget(20 * 0.1609344, 0.0, horizontreadmill::UrtmChecksum::ExcludeStx).toHex() == "02530214003303");
    EXPECT_TRUE(horizontreadmill::urtmTarget(9 * 0.1609344, 0.0, horizontreadmill::UrtmChecksum::ExcludeStx).toHex() == "02530209000403");
    EXPECT_TRUE(horizontreadmill::urtmTarget(20 * 0.1609344, 3.0, horizontreadmill::UrtmChecksum::ExcludeStx).toHex() == "025302141edd03");
    EXPECT_TRUE(horizontreadmill::urtmNativeName("urtm047"));
    EXPECT_TRUE(horizontreadmill::urtmNativeName("URTM024"));
    EXPECT_TRUE(!horizontreadmill::urtmNativeName("XURTM047"));
    EXPECT_TRUE(horizontreadmill::urtmStart().toHex() == "02530100000000000000000e03");
    EXPECT_TRUE(horizontreadmill::urtmStop(false).toHex() == "0253030c03");
    EXPECT_TRUE(horizontreadmill::urtmStop(true).toHex() == "02530a0703");
    double speed = -1, incline = -1;
    auto variant = horizontreadmill::UrtmChecksum::Unknown;
    auto status = QByteArray::fromHex("02510306000300000000000000000000000b00000000003203");
    EXPECT_TRUE(horizontreadmill::urtmStatus(status, speed, incline, variant));
    EXPECT_TRUE(qAbs(speed - 6 * 0.1609344) < 1e-9 && incline == 0.0);
    auto bad = status; bad[3] = 7;
    EXPECT_TRUE(!horizontreadmill::urtmStatus(bad, speed, incline, variant));
    EXPECT_TRUE(!horizontreadmill::urtmStatus(status.left(24), speed, incline, variant));
    bad = status; bad[2] = 2;
    EXPECT_TRUE(!horizontreadmill::urtmStatus(bad, speed, incline, variant));
    status = QByteArray::fromHex("0251031428610003002f000100000000000b00000000007503");
    EXPECT_TRUE(horizontreadmill::urtmStatus(status, speed, incline, variant));
    EXPECT_TRUE(qAbs(speed - 20 * 0.1609344) < 1e-9 && incline == 4.0);
    EXPECT_TRUE(horizontreadmill::urtmNativeCapabilities("URTM047", true, true, true, true));
    EXPECT_FALSE(horizontreadmill::urtmNativeCapabilities("URTM047", false, true, true, true));
    EXPECT_FALSE(horizontreadmill::urtmNativeCapabilities("URTM047", true, false, true, true));
    EXPECT_FALSE(horizontreadmill::urtmNativeCapabilities("URTM047", true, true, false, true));
    EXPECT_FALSE(horizontreadmill::urtmNativeCapabilities("URTM047", true, true, true, false));
    EXPECT_FALSE(horizontreadmill::urtmNativeCapabilities("OTHER", true, true, true, true));
}


namespace {
struct UrtmFlow {
    using H = horizontreadmill;
    bool subscribed = true, paired = false, stopping = false, unconfirmed = false, idle = false;
    double speed = -123, incline = -456;
    qint64 statusTime = 0, pendingSince = 0;
    H::UrtmChecksum checksum = H::UrtmChecksum::Unknown;
    volatile double requestSpeed = -1;
    double requestInclination = -100;
    int8_t requestStart = -1, requestStop = -1, requestPause = -1;
    bool native = true, ftms = true;
    qint64 now = 1000;
    std::vector<QByteArray> frames;
    int started = 0;
    H::UrtmControlRefs refs() {
        return {subscribed, paired, stopping, unconfirmed, idle, speed, incline, statusTime, pendingSince,
                checksum, requestSpeed, requestInclination, requestStart, requestStop, requestPause};
    }
    H::UrtmStatusResult status(const char *hex) {
        return H::urtmApplyStatus(refs(), QByteArray::fromHex(hex), now);
    }
    bool update() {
        return H::urtmControlStep(refs(), native, ftms, now,
            [this](const QByteArray &frame) { frames.push_back(frame); },
            [](const QString &) {}, [this]() { ++started; }, [](double v) { return v; });
    }
};
constexpr const char *idlePacket = "025100020903";
constexpr const char *fullPacket = "0251031428610003002f000100000000000b00000000007503";
}

TEST(Urtm047Flow, IdleDoesNotStartAndStartRequiresSubscriptionAndRequest) {
    UrtmFlow f;
    f.subscribed = false;
    EXPECT_EQ(f.status(idlePacket), UrtmFlow::H::UrtmStatusResult::Idle);
    EXPECT_FALSE(f.update()); // FTMS remains available while not subscribed.
    EXPECT_TRUE(f.frames.empty());
    EXPECT_DOUBLE_EQ(f.speed, -123);
    EXPECT_DOUBLE_EQ(f.incline, -456);
    f.subscribed = true;
    EXPECT_TRUE(f.update());
    EXPECT_TRUE(f.frames.empty());
    f.requestStart = 1;
    EXPECT_TRUE(f.update());
    ASSERT_EQ(f.frames.size(), 1u);
    EXPECT_EQ(f.frames[0].toHex(), QByteArray("02530100000000000000000e03"));
    EXPECT_TRUE(f.unconfirmed);
    EXPECT_FALSE(f.paired);
    EXPECT_EQ(f.started, 1);
}

TEST(Urtm047Flow, UnconfirmedStartBlocksRepeatAndFtmsButStopStillWorks) {
    UrtmFlow f;
    f.status(idlePacket); f.requestStart = 1; ASSERT_TRUE(f.update());
    f.now += 6000;
    f.requestStart = 1;
    EXPECT_TRUE(f.update());
    EXPECT_EQ(f.frames.size(), 1u);
    f.requestSpeed = 4;
    EXPECT_TRUE(f.update());
    EXPECT_EQ(f.frames.size(), 1u);
    f.requestStop = 1;
    EXPECT_TRUE(f.update());
    ASSERT_EQ(f.frames.size(), 2u);
    EXPECT_EQ(f.frames.back().toHex(), QByteArray("0253030c03"));
    EXPECT_EQ(f.requestSpeed, -1);
    EXPECT_TRUE(f.stopping);
}

TEST(Urtm047Flow, PairedStatusEnablesSingleCombinedTarget) {
    UrtmFlow f;
    f.status(idlePacket); f.requestStart = 1; ASSERT_TRUE(f.update());
    ASSERT_EQ(f.status(fullPacket), UrtmFlow::H::UrtmStatusResult::Full);
    EXPECT_FALSE(f.unconfirmed);
    f.requestInclination = 3;
    ASSERT_TRUE(f.update());
    ASSERT_EQ(f.frames.size(), 2u);
    EXPECT_EQ(f.frames.back().toHex(), QByteArray("025302141edd03"));
    EXPECT_FALSE(f.paired);
    f.requestSpeed = 2;
    ASSERT_TRUE(f.update());
    EXPECT_EQ(f.frames.size(), 2u);
}

TEST(Urtm047Flow, DisconnectClearsPendingAndRelearnsOnReconnect) {
    UrtmFlow f;
    f.status(idlePacket); f.requestStart = 1; ASSERT_TRUE(f.update());
    f.requestSpeed = 5;
    UrtmFlow::H::urtmReset(f.refs(), true);
    EXPECT_FALSE(f.subscribed);
    EXPECT_FALSE(f.unconfirmed);
    EXPECT_FALSE(f.idle);
    EXPECT_EQ(f.checksum, UrtmFlow::H::UrtmChecksum::Unknown);
    EXPECT_EQ(f.requestSpeed, -1);
    f.subscribed = true;
    ASSERT_EQ(f.status(idlePacket), UrtmFlow::H::UrtmStatusResult::Idle);
    ASSERT_TRUE(f.update());
    EXPECT_EQ(f.frames.size(), 1u); // Reconnect alone must not start.
    f.requestStart = 1;
    ASSERT_TRUE(f.update());
    EXPECT_EQ(f.frames.size(), 2u);
}

TEST(Urtm047Flow, MissingNativeUsesExistingFallbackWithoutNativeWrite) {
    UrtmFlow f;
    f.native = false;
    f.requestStart = 1;
    EXPECT_FALSE(f.update());
    EXPECT_TRUE(f.frames.empty());
    EXPECT_EQ(f.requestStart, 1);
}


TEST(Urtm047Flow, StaleStatusNeverSuppressesNativeStop) {
    UrtmFlow f; f.status(fullPacket); f.now += 6000;
    f.requestStop = f.requestStart = 1; f.requestSpeed = 4;
    ASSERT_TRUE(f.update()); ASSERT_EQ(f.frames.size(), 1u);
    EXPECT_EQ(f.frames[0], UrtmFlow::H::urtmStop(false));
    EXPECT_EQ(f.requestStart, -1); EXPECT_EQ(f.requestSpeed, -1);
}

TEST(Urtm047Flow, ExpiredIdleCannotAuthorizeStartOrFallback) {
    UrtmFlow f; f.status(idlePacket); f.now += 5000; f.requestStart = 1;
    EXPECT_TRUE(f.update()); EXPECT_TRUE(f.frames.empty()); EXPECT_EQ(f.requestStart, -1);
}

TEST(Urtm047Flow, ZeroSpeedStatusDoesNotConfirmStart) {
    UrtmFlow f; f.status(idlePacket); f.requestStart = 1; ASSERT_TRUE(f.update());
    auto zero = QByteArray::fromHex(fullPacket); zero[3] = 0;
    unsigned sum = 0; for (int i = 1; i < 23; ++i) sum += quint8(zero[i]);
    zero[23] = char((sum & 255) ^ 0x5a);
    ASSERT_EQ(UrtmFlow::H::urtmApplyStatus(f.refs(), zero, f.now), UrtmFlow::H::UrtmStatusResult::Full);
    EXPECT_TRUE(f.unconfirmed); EXPECT_FALSE(f.paired);
    f.requestStart = 1; ASSERT_TRUE(f.update()); EXPECT_EQ(f.frames.size(), 1u);
    f.requestSpeed = 4; ASSERT_TRUE(f.update()); EXPECT_EQ(f.frames.size(), 1u);
    f.now += 5000; ASSERT_TRUE(f.update()); EXPECT_EQ(f.requestSpeed, -1);
    EXPECT_EQ(f.frames.size(), 1u); // Deliberately no unproven start acknowledgement.
}

TEST(Urtm047Flow, NativeUnavailableFtmsStopPreemptsStartAndTargets) {
    UrtmFlow f; f.subscribed = false; f.requestStart = f.requestStop = 1;
    f.requestSpeed = 5; f.requestInclination = 3;
    EXPECT_FALSE(f.update()); EXPECT_EQ(f.requestStop, 1);
    EXPECT_EQ(f.requestStart, -1); EXPECT_EQ(f.requestSpeed, -1); EXPECT_EQ(f.requestInclination, -100);
}

TEST(Urtm047Flow, RunningStatusDoesNotAuthorizeAnotherStart) {
    UrtmFlow f; f.status(fullPacket); f.requestStart = 1;
    EXPECT_TRUE(f.update()); EXPECT_TRUE(f.frames.empty()); EXPECT_EQ(f.requestStart, -1);
}


TEST(Urtm047Flow, ExpiredIdleWithUnavailableNativeCannotFallThroughToFtmsStart) {
    UrtmFlow f; f.status(idlePacket); f.subscribed = false; f.now += 5000; f.requestStart = 1;
    EXPECT_TRUE(f.update()); EXPECT_TRUE(f.frames.empty()); EXPECT_EQ(f.requestStart, -1);
}


TEST(Urtm047Flow, NonNativeStopStartRequestsPreserveLegacyFallback) {
    UrtmFlow f; f.native = false; f.requestStart = f.requestStop = 1; f.requestSpeed = 5;
    EXPECT_FALSE(f.update()); EXPECT_EQ(f.requestStart, 1); EXPECT_EQ(f.requestStop, 1);
    EXPECT_EQ(f.requestSpeed, 5); EXPECT_TRUE(f.frames.empty());
}

TEST(Urtm047Flow, ZeroSpeedStatusReleasesLocalStopGate) {
    UrtmFlow f; f.status(idlePacket); f.requestStart = 1; ASSERT_TRUE(f.update());
    f.requestStop = 1; ASSERT_TRUE(f.update());
    auto zero = QByteArray::fromHex(fullPacket); zero[3] = 0;
    unsigned sum = 0; for (int i = 1; i < 23; ++i) sum += quint8(zero[i]);
    zero[23] = char((sum & 255) ^ 0x5a);
    ASSERT_EQ(UrtmFlow::H::urtmApplyStatus(f.refs(), zero, f.now), UrtmFlow::H::UrtmStatusResult::Full);
    EXPECT_FALSE(f.unconfirmed); EXPECT_FALSE(f.stopping); EXPECT_TRUE(f.paired);
    EXPECT_DOUBLE_EQ(f.speed, 0); EXPECT_DOUBLE_EQ(f.incline, 4);
    EXPECT_EQ(f.frames.size(), 2u);
}

TEST(Urtm047Flow, IdleAfterStopKeepsRestartBlocked) {
    UrtmFlow f; f.status(idlePacket); f.requestStart = 1; ASSERT_TRUE(f.update());
    f.requestStop = 1; ASSERT_TRUE(f.update());
    ASSERT_EQ(f.status(idlePacket), UrtmFlow::H::UrtmStatusResult::Idle);
    EXPECT_TRUE(f.stopping); EXPECT_TRUE(f.unconfirmed); EXPECT_FALSE(f.paired);
    f.requestStart = 1; ASSERT_TRUE(f.update());
    EXPECT_EQ(f.requestStart, -1); EXPECT_EQ(f.frames.size(), 2u);
}

TEST(Urtm047Flow, DelayedPreStopIdleCannotReleaseStoppingOrAuthorizeRestart) {
    UrtmFlow f; ASSERT_EQ(f.status(idlePacket), UrtmFlow::H::UrtmStatusResult::Idle);
    f.requestStart = 1; ASSERT_TRUE(f.update());
    ASSERT_EQ(f.status(fullPacket), UrtmFlow::H::UrtmStatusResult::Full);
    f.requestStop = 1; ASSERT_TRUE(f.update());
    f.now += 100;
    ASSERT_EQ(f.status(idlePacket), UrtmFlow::H::UrtmStatusResult::Idle);
    EXPECT_TRUE(f.stopping); EXPECT_FALSE(f.idle); EXPECT_FALSE(f.paired);
    f.requestStart = 1; f.requestSpeed = 4; ASSERT_TRUE(f.update());
    EXPECT_EQ(f.requestStart, -1); EXPECT_EQ(f.requestSpeed, -1);
    ASSERT_EQ(f.frames.size(), 2u);
    EXPECT_EQ(f.frames.back(), UrtmFlow::H::urtmStop(false, f.checksum));
}

TEST(Urtm047Flow, StopFallsBackToFtmsWhenStartUnconfirmedAndNativeUnavailable) {
    UrtmFlow f; f.status(idlePacket); f.requestStart = 1; ASSERT_TRUE(f.update());
    f.subscribed = false; f.requestStop = f.requestStart = 1;
    f.requestSpeed = 4; f.requestInclination = 2;
    EXPECT_FALSE(f.update()); // FTMS stop remains for the existing transport.
    EXPECT_EQ(f.requestStop, 1); EXPECT_EQ(f.requestStart, -1);
    EXPECT_EQ(f.requestSpeed, -1); EXPECT_EQ(f.requestInclination, -100);
    EXPECT_TRUE(f.stopping); EXPECT_EQ(f.frames.size(), 1u);
    f.requestStop = -1; // The FTMS adapter consumed the one-shot request.
    f.requestSpeed = 3;
    EXPECT_TRUE(f.update()); EXPECT_EQ(f.requestSpeed, -1); EXPECT_EQ(f.frames.size(), 1u);
}
