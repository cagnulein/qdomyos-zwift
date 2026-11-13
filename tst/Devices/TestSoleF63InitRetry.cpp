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

TEST(SoleF63InitRetryStateTest, IdentityProbeIsBoundedToThreeAttempts) {
    SoleF63InitRetryState state;
    ASSERT_TRUE(state.beginAfterNotifyCccdWrite(true, true, true));

    for (int attempt = 0; attempt < 3; ++attempt) {
        ASSERT_TRUE(state.shouldRetry());
        state.recordAttempt();
    }

    EXPECT_EQ(state.attemptCount(), 3);
    EXPECT_FALSE(state.shouldRetry());
    state.recordAttempt();
    EXPECT_EQ(state.attemptCount(), 3);
    EXPECT_FALSE(state.receiveNotification(QByteArray::fromHex("5b0210005d")));
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

TEST(SoleF63PassiveRepliesTest, AckOnlyInResponseToValidVendorNotification) {
    SoleF63PassiveReplies replies;
    EXPECT_TRUE(replies.nextReply(1000).isEmpty());
    replies.observe(QByteArray::fromHex("5b0f06023b00000000000a0001000000015d"), 1000);
    EXPECT_EQ(replies.nextReply(1001), QByteArray::fromHex("5b0400064f4b5d"));
    replies.observe(QByteArray::fromHex("5b0210005d"), 1100);
    EXPECT_EQ(replies.nextReply(1101), QByteArray::fromHex("5b0400104f4b5d"));
    replies.observe(QByteArray::fromHex("5b030901005d"), 1200);
    EXPECT_EQ(replies.nextReply(1201), QByteArray::fromHex("5b0400094f4b5d"));
    replies.observe(QByteArray::fromHex("5b0400064f4b5d"), 1300); // incoming ACK
    EXPECT_TRUE(replies.nextReply(1301).isEmpty());
}

TEST(SoleF63PassiveRepliesTest, StateEchoIsReceivedOnlyFreshAndSingleUse) {
    SoleF63PassiveReplies replies;
    replies.observe(QByteArray::fromHex("5b0203035d"), 1000);
    replies.observe(QByteArray::fromHex("5b0203045d"), 1100);
    EXPECT_EQ(replies.pendingCount(), 1);
    EXPECT_EQ(replies.nextReply(1101), QByteArray::fromHex("5b0203045d"));
    EXPECT_TRUE(replies.nextReply(1102).isEmpty());
    replies.observe(QByteArray::fromHex("5b0203065d"), 2000);
    EXPECT_TRUE(replies.nextReply(2901).isEmpty());
    replies.observe(QByteArray::fromHex("5b0203025d"), 3000); // unknown state
    EXPECT_TRUE(replies.nextReply(3001).isEmpty());
}

TEST(SoleF63PassiveRepliesTest, MalformedPacketNoAckAndRepeatedTelemetryCoalesces) {
    SoleF63PassiveReplies replies;
    replies.observe(QByteArray::fromHex("5b02030300"), 1000);
    replies.observe(QByteArray::fromHex("5b0f06023b00000000000a0001000000015d"), 1001);
    replies.observe(QByteArray::fromHex("5b0f06023b00000000000a0301000000015d"), 1002);
    EXPECT_EQ(replies.pendingCount(), 1);
    EXPECT_EQ(replies.nextReply(1003), QByteArray::fromHex("5b0400064f4b5d"));
    EXPECT_TRUE(replies.nextReply(1004).isEmpty());
    replies.observe(QByteArray::fromHex("5b08f0900000b40a0f125d"), 2000); // identity
    EXPECT_TRUE(replies.nextReply(2001).isEmpty());
    replies.reset();
    EXPECT_EQ(replies.pendingCount(), 0);
}
