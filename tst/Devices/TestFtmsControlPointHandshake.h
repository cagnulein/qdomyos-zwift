#pragma once

#include <gtest/gtest.h>

#include <QList>

#include "devices/ftmsbike/ftmscontrolpointhandshake.h"
#include "devices/ftmsbike/ftmsbike.h" // FTMS opcodes

namespace {

constexpr uint8_t kControl = FTMS_REQUEST_CONTROL; // 0x00
constexpr uint8_t kStart = FTMS_START_RESUME;      // 0x07

/**
 * Drives the handshake the way ftmsbike::update() does: a monotonic clock and a fixed poll,
 * recording every opcode that would have gone onto the wire.
 */
struct handshakeDriver {
    ftmsControlPointHandshake handshake;
    qint64 nowMs = 0;
    qint64 pollMs = 200;
    QList<QPair<qint64, uint8_t>> writes;

    /** Poll for the given duration without the bike ever answering. */
    void runSilent(qint64 durationMs) {
        const qint64 end = nowMs + durationMs;
        while (nowMs <= end) {
            pump();
            nowMs += pollMs;
        }
    }

    void pump() {
        uint8_t opcode = 0;
        if (handshake.nextCommand(nowMs, &opcode))
            writes.append(qMakePair(nowMs, opcode));
    }

    int countOf(uint8_t opcode) const {
        int n = 0;
        for (const auto &w : writes)
            if (w.second == opcode)
                n++;
        return n;
    }

    bool wrote(uint8_t opcode) const { return countOf(opcode) > 0; }
};

} // namespace

TEST(FtmsControlPointHandshakeTest, InertUntilBegun) {
    ftmsControlPointHandshake handshake;
    uint8_t opcode = 0xEE;

    EXPECT_FALSE(handshake.active());
    EXPECT_FALSE(handshake.done());
    EXPECT_FALSE(handshake.nextCommand(0, &opcode));
    EXPECT_EQ(0xEE, opcode); // untouched
}

TEST(FtmsControlPointHandshakeTest, StartsFromAZeroClock) {
    // The "not yet issued" state must not be inferred from a timestamp of 0, or a caller whose
    // clock starts at zero reissues the first command forever.
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, 0);

    d.pump();
    ASSERT_EQ(1, d.writes.size());
    EXPECT_EQ(kControl, d.writes.first().second);

    uint8_t outstanding = 0xEE;
    ASSERT_TRUE(d.handshake.pending(&outstanding));
    EXPECT_EQ(kControl, outstanding);

    d.pump(); // same millisecond, nothing owed
    EXPECT_EQ(1, d.writes.size());
}

TEST(FtmsControlPointHandshakeTest, StartWaitsForControlToBeGranted) {
    // The defect this class exists for: START_RESUME must not go out until REQUEST_CONTROL has
    // been answered, or the bike discards it and the console never counts down.
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);

    d.runSilent(900); // most of a second of polling, no answer
    EXPECT_TRUE(d.wrote(kControl));
    EXPECT_FALSE(d.wrote(kStart)) << "START_RESUME went out before control was granted";
    EXPECT_EQ(1, d.countOf(kControl)) << "reissued before the timeout expired";
}

TEST(FtmsControlPointHandshakeTest, AcknowledgementReleasesTheStart) {
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);
    d.pump();

    d.nowMs += 40; // a real bike answers in tens of ms
    d.handshake.onResponse(kControl, true, d.nowMs);
    d.pump();

    ASSERT_EQ(2, d.writes.size());
    EXPECT_EQ(kStart, d.writes.last().second);
    EXPECT_FALSE(d.handshake.done()) << "done before the start itself was acknowledged";

    d.nowMs += 40;
    d.handshake.onResponse(kStart, true, d.nowMs);
    EXPECT_TRUE(d.handshake.done());
    EXPECT_FALSE(d.handshake.degraded());
    EXPECT_FALSE(d.handshake.active());
}

TEST(FtmsControlPointHandshakeTest, UnansweredCommandIsRepeated) {
    // The old code issued once and latched. An unanswered command has to be tried again.
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);

    d.runSilent(1100);
    EXPECT_EQ(2, d.countOf(kControl));
    EXPECT_FALSE(d.wrote(kStart));
}

TEST(FtmsControlPointHandshakeTest, SilentBikeStillGetsStarted) {
    // The regression guard. Plenty of consoles never indicate on 0x2AD9 at all; waiting forever
    // for them would be a worse defect than the one being fixed. After the attempt budget the
    // machine proceeds blind - exactly what the old code did - and says so via degraded().
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);

    d.runSilent(10000);

    EXPECT_TRUE(d.wrote(kControl));
    EXPECT_TRUE(d.wrote(kStart)) << "a bike that never answers would never have been started";
    EXPECT_TRUE(d.handshake.done());
    EXPECT_TRUE(d.handshake.degraded());
}

TEST(FtmsControlPointHandshakeTest, SilentBikeCostsABoundedDelay) {
    // Three attempts at a one second timeout: the start must be out well inside four seconds,
    // and the whole handshake finished inside eight. Paid once per connection.
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);
    d.runSilent(10000);

    qint64 firstStart = -1;
    for (const auto &w : d.writes)
        if (w.second == kStart && firstStart < 0)
            firstStart = w.first;

    ASSERT_GE(firstStart, 0);
    EXPECT_LE(firstStart, 4000);
    EXPECT_TRUE(d.handshake.done());
}

TEST(FtmsControlPointHandshakeTest, RejectionIsRetriedNotLatched) {
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);
    d.pump();

    d.nowMs += 40;
    d.handshake.onResponse(kControl, false, d.nowMs); // CONTROL_NOT_PERMITTED and friends
    d.pump();

    EXPECT_EQ(2, d.countOf(kControl)) << "a rejection should be retried straight away";
    EXPECT_FALSE(d.wrote(kStart));
    EXPECT_FALSE(d.handshake.done());
}

TEST(FtmsControlPointHandshakeTest, ResponsesToOtherCommandsAreIgnored) {
    // Resistance writes are acknowledged on the same characteristic. Mistaking one for the
    // control grant would put START_RESUME back into the race this class removes.
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);
    d.pump();

    d.nowMs += 40;
    d.handshake.onResponse(FTMS_SET_TARGET_RESISTANCE_LEVEL, true, d.nowMs);
    d.pump();

    EXPECT_FALSE(d.wrote(kStart)) << "an unrelated acknowledgement advanced the handshake";

    uint8_t outstanding = 0xEE;
    ASSERT_TRUE(d.handshake.pending(&outstanding)) << "the control grant stopped being awaited";
    EXPECT_EQ(kControl, outstanding);
}

TEST(FtmsControlPointHandshakeTest, AFailedQueueingIsNotAnIssuedCommand) {
    // writeCharacteristic returns false when the service is not there. That is not a command in
    // flight, so it must not consume the wait or the attempt budget.
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);
    d.pump();
    EXPECT_EQ(1, d.writes.size());

    d.handshake.onWriteFailed();
    uint8_t outstanding = 0;
    EXPECT_FALSE(d.handshake.pending(&outstanding)) << "a failed write left a command in flight";

    d.pump(); // same instant: free to try again immediately
    EXPECT_EQ(2, d.countOf(kControl));
}

TEST(FtmsControlPointHandshakeTest, ResetMakesItInertAgain) {
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, d.nowMs);
    d.pump();

    d.handshake.reset();
    EXPECT_FALSE(d.handshake.active());
    EXPECT_FALSE(d.handshake.done());

    const int before = d.writes.size();
    d.runSilent(5000);
    EXPECT_EQ(before, d.writes.size()) << "a reset handshake still wrote to the bike";
}

TEST(FtmsControlPointHandshakeTest, AClockGoingBackwardsDoesNotStall) {
    handshakeDriver d;
    d.handshake.begin(kControl, kStart, 10000);

    uint8_t opcode = 0;
    ASSERT_TRUE(d.handshake.nextCommand(10000, &opcode));

    EXPECT_FALSE(d.handshake.nextCommand(5000, &opcode)); // jumped back, still owes nothing
    EXPECT_TRUE(d.handshake.nextCommand(11000, &opcode)); // forwards again, timeout honoured
    EXPECT_EQ(kControl, opcode);
}
