#include <gtest/gtest.h>

#include "solef80treadmill/solef80treadmill.h"

TEST(SoleF63InitRetryStateTest, OnlyStartsAfterTheCurrentF63NotifyCccdIsEnabled) {
    SoleF63InitRetryState state;

    EXPECT_FALSE(state.beginAfterNotifyCccdWrite(false, true, true));
    EXPECT_FALSE(state.shouldRetry());
    EXPECT_FALSE(state.beginAfterNotifyCccdWrite(true, false, true));
    EXPECT_FALSE(state.shouldRetry());
    EXPECT_FALSE(state.beginAfterNotifyCccdWrite(true, true, false));
    EXPECT_FALSE(state.shouldRetry());

    EXPECT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));
    EXPECT_TRUE(state.shouldRetry());
    EXPECT_EQ(state.attemptCount(), 0);
}

TEST(SoleF63InitRetryStateTest, RetriesUntilTheFirstNotificationArrives) {
    SoleF63InitRetryState state;
    ASSERT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));

    for (int attempt = 0; attempt < 1000; ++attempt) {
        ASSERT_TRUE(state.shouldRetry());
        state.recordAttempt();
    }

    EXPECT_EQ(state.attemptCount(), 1000);
    EXPECT_FALSE(state.receiveNotification(QByteArray::fromHex("5b0210005d")));
    EXPECT_TRUE(state.shouldRetry());
    EXPECT_TRUE(state.receiveNotification(QByteArray::fromHex("5b08f0900000b40a0f125d")));
    EXPECT_FALSE(state.shouldRetry());
    EXPECT_TRUE(state.takeResponse());
    EXPECT_FALSE(state.takeResponse());
}

TEST(SoleF63InitRetryStateTest, IgnoresNotificationsBeforeAnyInitWrite) {
    SoleF63InitRetryState state;
    ASSERT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));

    EXPECT_FALSE(state.receiveNotification(QByteArray::fromHex("5b08f0900000b40a0f125d")));
    EXPECT_TRUE(state.shouldRetry());
    EXPECT_FALSE(state.takeResponse());

    state.recordAttempt();
    EXPECT_TRUE(state.receiveNotification(QByteArray::fromHex("5b08f0900000b40a0f125d")));
    EXPECT_TRUE(state.takeResponse());
}

TEST(SoleF63InitRetryStateTest, DisconnectWhileRetryingRequiresNewCccdSuccess) {
    SoleF63InitRetryState state;
    ASSERT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));
    state.recordAttempt();
    state.cancel();

    EXPECT_FALSE(state.shouldRetry());
    EXPECT_EQ(state.attemptCount(), 0);
    EXPECT_FALSE(state.receiveNotification(QByteArray::fromHex("5b08f0900000b40a0f125d")));

    EXPECT_FALSE(state.beginAfterNotifyCccdWrite(true, false, true));
    EXPECT_FALSE(state.shouldRetry());
    EXPECT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));
    EXPECT_TRUE(state.shouldRetry());
}

TEST(SoleF63TelemetryGuardTest, RejectsCommandRepliesUntilRealTelemetryArrives) {
    SoleF63TelemetryGuard guard;
    EXPECT_FALSE(guard.isFresh(1000));
    EXPECT_FALSE(guard.recordVendorPacket(QByteArray::fromHex("5b0203015d"), 1000));
    EXPECT_FALSE(guard.recordVendorPacket(QByteArray::fromHex("5b0400f14f4b5d"), 1100));
    EXPECT_FALSE(guard.isFresh(1200));

    QByteArray malformed(18, char(0));
    EXPECT_FALSE(guard.recordVendorPacket(malformed, 1500));
    QByteArray telemetry(18, char(0));
    telemetry[0] = char(0x5b);
    telemetry[1] = char(0x0f);
    telemetry[2] = char(0x06);
    telemetry[17] = char(0x5d);
    EXPECT_TRUE(guard.recordVendorPacket(telemetry, 2000));
    EXPECT_TRUE(guard.isFresh(6999));
    EXPECT_TRUE(guard.isFresh(7000));
    EXPECT_FALSE(guard.isFresh(7001));
    EXPECT_FALSE(guard.isFresh(1999));

    guard.reset();
    EXPECT_FALSE(guard.isFresh(2001));
}

TEST(SoleF63NativeProtocolTest, StartSequenceMatchesSoleFitnessCapture) {
    const auto start = SoleF63NativeProtocol::startFrames();
    ASSERT_EQ(start.size(), 15);
    EXPECT_EQ(start.first(), QByteArray::fromHex("5b06070123009baa5d"));
    EXPECT_EQ(start.at(1), QByteArray::fromHex("5b030820025d"));
    EXPECT_EQ(start.at(2), QByteArray::fromHex("5b0504000000005d"));
    EXPECT_EQ(start.at(5), QByteArray::fromHex("5b0202025d"));
    EXPECT_EQ(start.at(8), QByteArray::fromHex("5b0400404f4b5d"));
    EXPECT_EQ(start.at(9), QByteArray::fromHex("5b0400404f4b5d"));
    for (int i = 12; i < 15; ++i)
        EXPECT_EQ(start.at(i), QByteArray::fromHex("5b0203035d"));

    // No speculative F1 stop or training-reset requests in native Start.
    for (const auto &frame : start) {
        EXPECT_NE(frame, QByteArray::fromHex("5b02f1065d"));
        EXPECT_NE(frame, QByteArray::fromHex("5b0203005d"));
        EXPECT_NE(frame, QByteArray::fromHex("5b0400f14f4b5d"));
    }
}

TEST(SoleF63NativeProtocolTest, StopUsesOnlyCapturedPhysicalStopFrames) {
    const auto stop = SoleF63NativeProtocol::stopFrames();
    ASSERT_EQ(stop.size(), 3);
    for (const auto &frame : stop)
        EXPECT_EQ(frame, QByteArray::fromHex("5b0203065d"));
}

TEST(SoleF63TelemetryGuardTest, RejectsNonTelemetryEvenWithValidDelimiters) {
    SoleF63TelemetryGuard guard;
    QByteArray wrongHeader(18, char(0));
    wrongHeader[0] = char(0x5b);
    wrongHeader[17] = char(0x5d);
    EXPECT_FALSE(guard.recordVendorPacket(wrongHeader, 300));
    EXPECT_FALSE(guard.isFresh(301));
    EXPECT_TRUE(guard.recordVendorPacket(QByteArray::fromHex("5b0f06023b00000000000a0001000000015d"), 400));
    EXPECT_TRUE(guard.isFresh(401));
}
