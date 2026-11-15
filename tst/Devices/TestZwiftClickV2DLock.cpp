#include <gtest/gtest.h>
#include "zwift_play/zwiftclickremote.h"

TEST(ZwiftClickV2DLockTest, ExtractsOnlyCompleteFF0300Challenge) {
    const QByteArray challenge = QByteArray::fromHex("ff03000a03ff0080");
    EXPECT_EQ(zwiftclickremote::dLockChallengePayload(challenge),
              QByteArray::fromHex("0a03ff0080"));
    EXPECT_TRUE(zwiftclickremote::dLockChallengePayload(QByteArray::fromHex("ff0300")).isEmpty());
    EXPECT_TRUE(zwiftclickremote::dLockChallengePayload(QByteArray::fromHex("ff0400aa")).isEmpty());
    EXPECT_TRUE(zwiftclickremote::dLockChallengePayload(QByteArray::fromHex("230300aa")).isEmpty());
    EXPECT_TRUE(zwiftclickremote::dLockChallengePayload(QByteArray::fromHex("ff0301aa")).isEmpty());
}

TEST(ZwiftClickV2DLockTest, PrependsSignedReplyWithoutChangingBytes) {
    const QByteArray response = QByteArray::fromHex("00aabbff03");
    EXPECT_EQ(zwiftclickremote::dLockResponseFrame(response),
              QByteArray::fromHex("ff040000aabbff03"));
    EXPECT_EQ(zwiftclickremote::dLockResponseFrame(QByteArray()),
              QByteArray::fromHex("ff0400"));
}

TEST(ZwiftClickV2DLockTest, UsesRecentRideOnEchoToAvoidUnneededLogin) {
    EXPECT_TRUE(zwiftclickremote::needsDLockReply(0, 10000));
    EXPECT_TRUE(zwiftclickremote::needsDLockReply(5000, 10000));
    EXPECT_FALSE(zwiftclickremote::needsDLockReply(8100, 10000));
    EXPECT_FALSE(zwiftclickremote::needsDLockReply(10050, 10000));
    EXPECT_TRUE(zwiftclickremote::needsDLockReply(10000, 15000));
}
