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
