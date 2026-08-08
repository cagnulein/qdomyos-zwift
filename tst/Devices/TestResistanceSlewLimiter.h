#pragma once

#include <gtest/gtest.h>

#include <QList>
#include <QPair>

#include "devices/ftmsbike/resistanceslewlimiter.h"

namespace {

/**
 * Drives the limiter the way ftmsbike::update() does: a monotonic clock, a fixed poll
 * interval, and a target that is handed in once and then only ticked.
 */
struct slewDriver {
    resistanceSlewLimiter limiter;
    qint64 nowMs = 0;
    qint64 pollMs = 200;
    QList<QPair<qint64, resistance_t>> commands;

    void run(resistance_t target, qint64 durationMs) {
        limiter.setTarget(target, nowMs);
        const qint64 end = nowMs + durationMs;
        while (nowMs <= end) {
            resistance_t next;
            if (limiter.step(nowMs, &next))
                commands.append(qMakePair(nowMs, next));
            nowMs += pollMs;
        }
    }

    resistance_t last() const { return commands.isEmpty() ? -1 : commands.last().second; }

    /**
     * The property the limiter exists for: at no point has the commanded level
     * travelled further from where the ramp started than the actuator could have in
     * the same time. Instantaneous rate between two commands is the wrong measure -
     * levels are integers, so a single level always lands slightly early or late -
     * but the distance covered is exact.
     */
    bool neverAhead(qint64 startMs, resistance_t startLevel, double rate) const {
        for (const auto &command : commands) {
            const double travelled = qAbs((double)command.second - (double)startLevel);
            const double reachable = rate * (double)(command.first - startMs) / 1000.0;
            if (travelled > reachable + 1e-9)
                return false;
        }
        return true;
    }
};

} // namespace

TEST(ResistanceSlewLimiterTest, DisabledByDefaultAndWithZeroRates) {
    resistanceSlewLimiter limiter;
    EXPECT_FALSE(limiter.enabled());

    limiter.configure(0, 0);
    EXPECT_FALSE(limiter.enabled());

    limiter.configure(2, 0);
    EXPECT_TRUE(limiter.enabled());

    limiter.configure(0, 4);
    EXPECT_TRUE(limiter.enabled());

    // Negative values are nonsense from a settings field: treat them as off.
    limiter.configure(-1, -1);
    EXPECT_FALSE(limiter.enabled());
}

TEST(ResistanceSlewLimiterTest, UpwardJumpIsSpreadAtTheConfiguredRate) {
    slewDriver driver;
    driver.limiter.configure(2, 4);
    driver.limiter.sync(1);

    // 1 -> 20 is 19 levels; at 2 levels/s that is 9,5 s, so it is not there at 9 s.
    driver.run(20, 9000);
    EXPECT_LT(driver.last(), 20);
    EXPECT_TRUE(driver.neverAhead(0, 1, 2.0));

    driver.run(20, 1000);
    EXPECT_EQ(driver.last(), 20);
    EXPECT_TRUE(driver.neverAhead(0, 1, 2.0));
}

TEST(ResistanceSlewLimiterTest, DownwardJumpUsesTheDownwardRate) {
    slewDriver driver;
    driver.limiter.configure(2, 4);
    driver.limiter.sync(20);

    // 20 -> 1 is 19 levels; at 4 levels/s that is 4,75 s...
    driver.run(1, 4000);
    EXPECT_GT(driver.last(), 1);
    EXPECT_TRUE(driver.neverAhead(0, 20, 4.0));

    // ...so it arrives at 5 s, which the upward rate would not have managed.
    driver.run(1, 1000);
    EXPECT_EQ(driver.last(), 1);
    EXPECT_TRUE(driver.neverAhead(0, 20, 4.0));
}

TEST(ResistanceSlewLimiterTest, RateSurvivesAPollShorterThanOneLevel) {
    slewDriver driver;
    driver.limiter.configure(2, 4);
    driver.limiter.sync(0);

    // 200 ms of poll against 500 ms per level. The leftover fraction has to carry
    // across polls, otherwise the ramp silently runs at 1,67 levels/s instead of 2.
    driver.run(100, 20000);

    EXPECT_TRUE(driver.neverAhead(0, 0, 2.0));
    EXPECT_GE(driver.last(), 39);
    EXPECT_LE(driver.last(), 40);
}

TEST(ResistanceSlewLimiterTest, RampContinuesWithoutFurtherDemand) {
    slewDriver driver;
    driver.limiter.configure(2, 4);
    driver.limiter.sync(5);

    // One target, then nothing but ticks. This is the case that breaks if the pending
    // target does not outlive the request that produced it.
    driver.run(15, 6000);

    EXPECT_GT(driver.commands.size(), 1);
    EXPECT_EQ(driver.limiter.lastCommanded(), 15);
    EXPECT_FALSE(driver.limiter.pending());
}

TEST(ResistanceSlewLimiterTest, RetargetingMidRampDoesNotStallTheRamp) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 4);
    limiter.sync(0);

    // ERG re-issues a target about once a second. If each one restarted the clock, a
    // limiter whose step is shorter than that interval would never move at all.
    resistance_t next;
    int writes = 0;
    for (qint64 now = 0; now <= 5000; now += 200) {
        if (now % 1000 == 0)
            limiter.setTarget(40, now);
        if (limiter.step(now, &next))
            writes++;
    }

    EXPECT_GT(writes, 5);
    EXPECT_GE(limiter.lastCommanded(), 9);
    EXPECT_LE(limiter.lastCommanded(), 10);
}

TEST(ResistanceSlewLimiterTest, IdleTimeDoesNotBuyAJump) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 4);
    limiter.sync(10);

    // Sitting at 10 for a minute must not entitle the next target to arrive at once.
    limiter.setTarget(20, 60000);

    resistance_t next;
    EXPECT_FALSE(limiter.step(60000, &next));
    ASSERT_TRUE(limiter.step(60500, &next));
    EXPECT_EQ(next, 11);
}

TEST(ResistanceSlewLimiterTest, StalledPollingDoesNotBuyAJumpEither) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 4);
    limiter.sync(0);
    limiter.setTarget(60, 0);

    // A 30 s gap in the polling. The magnets were not moving during it either, so the
    // catch-up is capped instead of being paid out in one command.
    resistance_t next;
    ASSERT_TRUE(limiter.step(30000, &next));
    EXPECT_LE(next, 4);
}

TEST(ResistanceSlewLimiterTest, UnlimitedDirectionPassesStraightThrough) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 0); // up limited, down free
    limiter.sync(30);

    resistance_t next;
    limiter.setTarget(5, 0);
    ASSERT_TRUE(limiter.step(0, &next));
    EXPECT_EQ(next, 5);

    limiter.setTarget(25, 0);
    EXPECT_FALSE(limiter.step(0, &next));
}

TEST(ResistanceSlewLimiterTest, TargetAlreadyReachedProducesNoCommand) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 4);
    limiter.sync(12);

    resistance_t next;
    limiter.setTarget(12, 0);
    EXPECT_FALSE(limiter.pending());
    EXPECT_FALSE(limiter.step(0, &next));
    EXPECT_FALSE(limiter.step(10000, &next));
}

TEST(ResistanceSlewLimiterTest, FirstCommandWithoutHistoryIsNotLimited) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 4);
    EXPECT_FALSE(limiter.primed());

    // Nothing has told the limiter where the magnets are, so there is no distance to
    // measure a rate over and the target goes out as-is.
    resistance_t next;
    limiter.setTarget(18, 0);
    ASSERT_TRUE(limiter.step(0, &next));
    EXPECT_EQ(next, 18);
    EXPECT_TRUE(limiter.primed());
}

TEST(ResistanceSlewLimiterTest, SyncAdoptsAnExternallyCommandedLevel) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 4);
    limiter.sync(10);
    limiter.setTarget(30, 0);

    resistance_t next;
    ASSERT_TRUE(limiter.step(1000, &next));
    EXPECT_EQ(next, 12);

    // A resync - auto resistance switched off, for instance - drops the pending ramp.
    limiter.sync(3);
    EXPECT_FALSE(limiter.pending());
    EXPECT_EQ(limiter.lastCommanded(), 3);
    EXPECT_FALSE(limiter.step(20000, &next));
}

TEST(ResistanceSlewLimiterTest, ClockGoingBackwardsDoesNotEmitACommand) {
    resistanceSlewLimiter limiter;
    limiter.configure(2, 4);
    limiter.sync(5);
    limiter.setTarget(25, 10000);

    resistance_t next;
    EXPECT_FALSE(limiter.step(9000, &next));
    EXPECT_EQ(limiter.lastCommanded(), 5);
}

TEST(ResistanceSlewLimiterTest, MeasuredActuatorRatesReachTheMeasuredDurations) {
    // The rates the plan starts from - 2 levels/s up, 4 down - over the 19-level span
    // the console was measured across: 6,3-9,5 s up and 3,8-4,8 s down.
    slewDriver driver;
    driver.limiter.configure(2, 4);
    driver.limiter.sync(1);

    driver.run(20, 9400);
    EXPECT_LT(driver.last(), 20);
    driver.run(20, 400);
    EXPECT_EQ(driver.last(), 20);

    driver.run(1, 4600);
    EXPECT_GT(driver.last(), 1);
    driver.run(1, 400);
    EXPECT_EQ(driver.last(), 1);
}
