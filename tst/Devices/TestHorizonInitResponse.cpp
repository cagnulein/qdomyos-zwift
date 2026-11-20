#include <gtest/gtest.h>
#include <QByteArray>

#include "devices/horizontreadmill/horizontreadmill.h"

TEST(HorizonInitResponseTest, AcceptsExpectedProfileRepliesFromSuccessfulSevenStageHandshake) {
    // The first five replies carry the next user-profile index at byte 10.
    const char *profileReplies[] = {
        "55aa00000116db025d570175", // user1
        "55aa00000116db0213870275", // user2
        "55aa00000116db026e1b0375", // user3
        "55aa00000116db023c9e0475", // user4
        "55aa00000116db0246c40575"  // user5
    };
    for (uint8_t stage = 1; stage <= 5; ++stage) {
        const QByteArray packet = QByteArray::fromHex(profileReplies[stage - 1]);
        EXPECT_TRUE(horizontreadmill::isExpectedInitPacket(packet, stage));
        if (stage < 5)
            EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(packet, stage + 1));
    }

    const QByteArray profileTransition =
        QByteArray::fromHex("55aa0000011acb02cf1d7573657231");
    EXPECT_TRUE(horizontreadmill::isExpectedInitPacket(profileTransition, 6));
    EXPECT_TRUE(horizontreadmill::isExpectedInitPacket(profileTransition, 7));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(profileTransition, 8));
}

TEST(HorizonInitResponseTest, RepeatedUser5CannotAdvanceTheFailedHandshake) {
    // In Eric's failed capture this is repeated while QZ previously considered
    // init stages 6 and 8 successful without any control acknowledgement.
    const QByteArray staleUser5 =
        QByteArray::fromHex("55aa0000011acb0215197573657235");
    const QByteArray oldProfile5 =
        QByteArray::fromHex("55aa00000116db0246c4057573657235");
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(oldProfile5, 6));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(staleUser5, 8));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(oldProfile5, 8));
}

TEST(HorizonInitResponseTest, FinalStageRequiresAcknowledgeOfControlCommand10) {
    // Successful capture: 03/01 is acknowledged, then 03/10 is acknowledged.
    const QByteArray control01Ack =
        QByteArray::fromHex("55aa0000030002003e2e0100");
    const QByteArray control10Ack =
        QByteArray::fromHex("55aa0000030002007c1e1000");
    const QByteArray control02Ack =
        QByteArray::fromHex("55aa0000030002006d7b0200");
    const QByteArray status14 =
        QByteArray::fromHex("55aa000003140a0039e100000000000000000000");

    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(control01Ack, 8));
    EXPECT_TRUE(horizontreadmill::isExpectedInitPacket(control10Ack, 8));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(control02Ack, 8));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(status14, 8));
}

TEST(HorizonInitResponseTest, RejectsTruncatedOrUnrelatedPackets) {
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(QByteArray(), 8));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(
        QByteArray::fromHex("55aa0000030002007c1e10"), 8));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(
        QByteArray::fromHex("54aa0000030002007c1e1000"), 8));
    EXPECT_FALSE(horizontreadmill::isExpectedInitPacket(
        QByteArray::fromHex("55aa0000030002007c1e1000"), 0));
}
