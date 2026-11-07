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
    EXPECT_TRUE(state.receiveNotification());
    EXPECT_FALSE(state.shouldRetry());
    EXPECT_TRUE(state.takeResponse());
    EXPECT_FALSE(state.takeResponse());
}

TEST(SoleF63InitRetryStateTest, IgnoresNotificationsBeforeAnyInitWrite) {
    SoleF63InitRetryState state;
    ASSERT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));

    EXPECT_FALSE(state.receiveNotification());
    EXPECT_TRUE(state.shouldRetry());
    EXPECT_FALSE(state.takeResponse());

    state.recordAttempt();
    EXPECT_TRUE(state.receiveNotification());
    EXPECT_TRUE(state.takeResponse());
}

TEST(SoleF63InitRetryStateTest, DisconnectWhileRetryingRequiresNewCccdSuccess) {
    SoleF63InitRetryState state;
    ASSERT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));
    state.recordAttempt();
    state.cancel();

    EXPECT_FALSE(state.shouldRetry());
    EXPECT_EQ(state.attemptCount(), 0);
    EXPECT_FALSE(state.receiveNotification());

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
    telemetry[17] = char(0x5d);
    EXPECT_TRUE(guard.recordVendorPacket(telemetry, 2000));
    EXPECT_TRUE(guard.isFresh(6999));
    EXPECT_TRUE(guard.isFresh(7000));
    EXPECT_FALSE(guard.isFresh(7001));
    EXPECT_FALSE(guard.isFresh(1999));

    guard.reset();
    EXPECT_FALSE(guard.isFresh(2001));
}
