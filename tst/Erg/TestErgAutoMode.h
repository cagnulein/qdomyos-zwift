#pragma once

#include <gtest/gtest.h>

#include "ergautomode.h"

/**
 * The cases are taken from the control point writes MyWhoosh made in the ride of 7/8, so the
 * timings here are the real ones: targets every ~5 s, simulation writes in bursts of three at a
 * transition, and 673 requests for 0 W scattered through the coasting.
 */
TEST(ErgAutoModeTest, StartsOff) {
    ergAutoMode m;
    EXPECT_FALSE(m.ergShouldBeOn(0));
    EXPECT_FALSE(m.everSawTarget());
}

TEST(ErgAutoModeTest, ATargetTurnsItOn) {
    ergAutoMode m;
    m.onTargetPower(198, 10000);
    EXPECT_TRUE(m.ergShouldBeOn(10000));
    EXPECT_EQ(198, m.targetWatts());
}

TEST(ErgAutoModeTest, SimulationTurnsItOff) {
    // 19:37:10 in the log: the app goes back to slope, and ERG has to let go of the resistance.
    ergAutoMode m;
    m.onTargetPower(198, 10000);
    m.onSimulation(20000);
    EXPECT_FALSE(m.ergShouldBeOn(20000));
}

TEST(ErgAutoModeTest, AZeroTargetIsNoTargetNotZeroWatts) {
    // The case that makes "last opcode wins" wrong on its own: 673 of the requests in that ride
    // were for 0 W. Treating them as a power target would latch ERG on through every coast.
    ergAutoMode m;
    m.onTargetPower(198, 10000);
    ASSERT_TRUE(m.ergShouldBeOn(10000));

    m.onTargetPower(0, 20000);
    EXPECT_FALSE(m.ergShouldBeOn(20000)) << "a zero target kept ERG engaged";
}

TEST(ErgAutoModeTest, TargetsGoingQuietReleaseIt) {
    // Targets arrive every ~5 s during a block. Three missed in a row means the block ended or
    // the app went away; holding the last resistance forever would be the worst outcome.
    ergAutoMode m;
    m.onTargetPower(198, 10000);
    EXPECT_TRUE(m.ergShouldBeOn(10000 + 14000));
    EXPECT_FALSE(m.ergShouldBeOn(10000 + 16000)) << "a stale target still counted as live";
}

TEST(ErgAutoModeTest, ANormalBlockStaysOnAcrossUpdates) {
    ergAutoMode m;
    qint64 t = 10000;
    m.onTargetPower(93, t);
    for (int i = 0; i < 20; i++) { // five second cadence, as in the log
        t += 5000;
        m.onTargetPower(93, t);
        ASSERT_TRUE(m.ergShouldBeOn(t)) << "dropped out mid block at t=" << t;
        ASSERT_TRUE(m.ergShouldBeOn(t + 4000)) << "dropped out between updates at t=" << t;
    }
}

TEST(ErgAutoModeTest, ABurstOfSimulationWritesIsOneTransition) {
    // 19:37:10 delivered three simulation writes inside 200 ms. Toggling per write would thrash
    // the resistance motor; the hold makes the burst a single change.
    ergAutoMode m;
    m.onTargetPower(198, 10000);
    m.onSimulation(20000);
    m.onSimulation(20100);
    m.onSimulation(20200);
    EXPECT_FALSE(m.ergShouldBeOn(20200));

    // And an immediate target does not flip it straight back inside the hold window.
    m.onTargetPower(198, 20300);
    EXPECT_FALSE(m.ergShouldBeOn(20300)) << "flipped back inside the minimum hold";

    m.onTargetPower(198, 24000);
    EXPECT_TRUE(m.ergShouldBeOn(24000)) << "never recovered after the hold expired";
}

TEST(ErgAutoModeTest, TheHoldDoesNotBlockTheVeryFirstEngagement) {
    ergAutoMode m;
    m.onTargetPower(198, 500000);
    EXPECT_TRUE(m.ergShouldBeOn(500000)) << "the first target was swallowed by the hold";
}

TEST(ErgAutoModeTest, ResetForgetsEverything) {
    ergAutoMode m;
    m.onTargetPower(198, 10000);
    m.reset();
    EXPECT_FALSE(m.ergShouldBeOn(10000));
    EXPECT_FALSE(m.everSawTarget());
    EXPECT_EQ(0, m.targetWatts());
}

TEST(ErgAutoModeTest, AClockGoingBackwardsDoesNotTearDownASession) {
    ergAutoMode m;
    m.onTargetPower(198, 100000);
    EXPECT_TRUE(m.ergShouldBeOn(50000)) << "a backwards clock ended a live ERG block";
}

TEST(ErgAutoModeTest, TheRealTransitionsFromTheSevenAugustRide) {
    // The sequence as logged, in seconds from 19:35:07.
    ergAutoMode m;
    const qint64 s = 1000;
    m.onSimulation(0 * s);        // 19:35:07 free ride
    m.onSimulation(22 * s);       // 19:35:29
    EXPECT_FALSE(m.ergShouldBeOn(30 * s));

    m.onTargetPower(93, 38 * s);  // 19:35:45 block starts
    EXPECT_TRUE(m.ergShouldBeOn(38 * s));
    for (qint64 t = 43; t <= 118; t += 5)
        m.onTargetPower(93, t * s);
    EXPECT_TRUE(m.ergShouldBeOn(118 * s)) << "block did not hold to 19:37:05";

    m.onSimulation(123 * s);      // 19:37:10 back to free ride
    EXPECT_FALSE(m.ergShouldBeOn(123 * s));
    EXPECT_FALSE(m.ergShouldBeOn(154 * s)) << "re-engaged with no target in sight";

    m.onTargetPower(102, 163 * s); // 19:37:50 next block
    EXPECT_TRUE(m.ergShouldBeOn(163 * s));
}
