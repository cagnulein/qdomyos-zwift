#pragma once

#include <gtest/gtest.h>

#include "devices/renphobike/renphobike.h"

// These regression tests come from a real debug log attached to
// https://github.com/cagnulein/qdomyos-zwift/issues/4873 (RENPHO R-Q002 physical
// resistance knob). The user reported that turning the knob should move QZ's virtual
// gear, and separately that the knob was accidentally bumped earlier in the ride
// without them noticing. Both behaviors are exercised below: a real knob movement must
// move the gear, while a single noisy/glitched BLE sample must not.

TEST(RenphoKnobGearTrackerTest, FirstSampleEstablishesBaselineWithoutMovingGear) {
    renphobike::KnobGearTracker tracker;

    EXPECT_EQ(tracker.feed(9.5), 0);
    EXPECT_DOUBLE_EQ(tracker.stableResistance, 9.5);
}

TEST(RenphoKnobGearTrackerTest, SingleGlitchSampleIsIgnored) {
    // Logged sequence (line 27932-27968 of the attached log): the bike reports a steady
    // 9.5 baseline, then a single "13" sample (the knob being bumped, per the reporter),
    // immediately followed by a return towards a different value. Because "13" is never
    // confirmed by a second consecutive identical sample, it must never move the gear.
    renphobike::KnobGearTracker tracker;

    ASSERT_EQ(tracker.feed(9.5), 0);
    ASSERT_EQ(tracker.feed(9.5), 0);
    EXPECT_EQ(tracker.feed(13.0), 0);
    EXPECT_DOUBLE_EQ(tracker.stableResistance, 9.5);
}

TEST(RenphoKnobGearTrackerTest, PurelyAlternatingNoiseNeverMovesTheGear) {
    // A knob that is repeatedly bumped back and forth (no two consecutive samples equal)
    // must never be interpreted as a deliberate gear change.
    renphobike::KnobGearTracker tracker;
    const double noisySamples[] = {9.5, 13.0, 9.5, 13.0, 9.5, 13.0};

    int totalSteps = 0;
    for (double sample : noisySamples)
        totalSteps += tracker.feed(sample);

    EXPECT_EQ(totalSteps, 0);
}

TEST(RenphoKnobGearTrackerTest, RealKnobMoveFromLogTriggersGearUpOnceStable) {
    // Logged sequence (line 27932-28005): baseline 9.5, a single "13" glitch, then the
    // knob settles on "15" for many samples in a row. The gear must move up exactly once,
    // the first time "15" is confirmed by a second consecutive sample - not again for
    // every subsequent duplicate reading.
    renphobike::KnobGearTracker tracker;
    const double loggedSamples[] = {9.5, 9.5, 9.5, 13.0, 15.0, 15.0, 15.0, 15.0, 15.0};

    int totalSteps = 0;
    int nonZeroEvents = 0;
    for (double sample : loggedSamples) {
        int steps = tracker.feed(sample);
        totalSteps += steps;
        if (steps != 0)
            nonZeroEvents++;
    }

    EXPECT_EQ(nonZeroEvents, 1);
    EXPECT_EQ(totalSteps, 5);
    EXPECT_DOUBLE_EQ(tracker.stableResistance, 14.5);
}

TEST(RenphoKnobGearTrackerTest, RealKnobMoveDownFromLogTriggersGearDownOnceStable) {
    // Logged sequence (line 30172-30521): the resistance decreases from ~31 back down
    // towards 15, with several non-monotonic blips along the way (27.5->28, 24.5->25)
    // that must not register as gear-up moves. The first value that is actually
    // confirmed twice in a row ("18.5") is where the gear finally moves down.
    renphobike::KnobGearTracker tracker;
    tracker.feed(31.0);
    tracker.feed(31.0); // baseline confirmed at 31

    const double loggedSamples[] = {31.5, 31.5, 28.0, 27.5, 28.0, 25.0, 24.5,
                                     25.0, 22.0, 21.5, 19.0, 18.5, 18.5,
                                     17.0, 16.5, 15.5, 15.0};

    int totalSteps = 0;
    int nonZeroEvents = 0;
    for (double sample : loggedSamples) {
        int steps = tracker.feed(sample);
        totalSteps += steps;
        if (steps != 0)
            nonZeroEvents++;
    }

    EXPECT_EQ(nonZeroEvents, 1);
    EXPECT_EQ(totalSteps, -12);
    EXPECT_DOUBLE_EQ(tracker.stableResistance, 19.0);
}

TEST(RenphoKnobGearTrackerTest, FractionalRemainderCarriesOverToNextMovement) {
    // stepSize is 1.0 (one virtual gear per whole resistance unit); a 0.5 remainder from
    // a half-step knob movement must not be lost, it should accumulate into the next move.
    renphobike::KnobGearTracker tracker;
    tracker.feed(10.0);
    tracker.feed(10.0);

    EXPECT_EQ(tracker.feed(10.5), 0); // first sample of a new value, not confirmed yet
    EXPECT_EQ(tracker.feed(10.5), 0); // confirmed, but delta 0.5 truncates to 0 whole steps
    EXPECT_DOUBLE_EQ(tracker.stableResistance, 10.0);

    EXPECT_EQ(tracker.feed(11.0), 0);
    EXPECT_EQ(tracker.feed(11.0), 1); // now 1.0 away from the still-unmoved baseline, crosses a full step
    EXPECT_DOUBLE_EQ(tracker.stableResistance, 11.0);
}
