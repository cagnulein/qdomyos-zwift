#pragma once

#include <gtest/gtest.h>

#include "devices/renphobike/renphobike.h"

// These regression tests come from two real debug logs attached to
// https://github.com/cagnulein/qdomyos-zwift/issues/4873 (RENPHO R-Q002 physical resistance
// knob, ridden with ROUVY as the FTMS accessory).
//
// The first log (a short, steady-cadence test) led to a first version of this feature that
// inferred gear steps from raw resistance-level deltas. The second log (a full ~86 minute
// ride) showed that approach was fundamentally broken: the RENPHO's own reported resistance
// also moves with speed/effort at a constant simulated grade (resistance-vs-watt correlation
// measured at r=0.73 across the log), so a single burst of pedaling could be misread as the
// user spinning the knob through 25+ gear steps while the physical dial never left 1-20.
//
// The fix replaces delta-tracking with an *expected resistance* model: renphobike computes a
// deterministic, gear-independent resistance from the raw ROUVY grade (autoResistanceFromSlope,
// mirroring CharacteristicWriteProcessor::changeSlope()'s formula) and writes
// autoResistance*difficult + gearsModifier() directly to the bike instead of forwarding the raw
// slope. ResistanceReconciler then only trusts a *sustained disagreement* between that known
// target and the bike's actual reported resistance as a genuine knob move, and reconciles gears
// against it exactly.

TEST(RenphoAutoResistanceFromSlopeTest, MatchesRealRideLogValues) {
    // Logged at 17:23:29-17:23:30 (line 69897/69930 of the second log): grade 751 with
    // gears=8 produced target_resistance=24, i.e. an auto/grade-only resistance of 16.
    EXPECT_DOUBLE_EQ(renphobike::autoResistanceFromSlope(751, 0, 0, 0.0, 0.0), 16.0);

    // Logged at 17:23:32 (line 70078): grade changed slightly to 754, still bucketing to the
    // same auto resistance of 16 (751*1.5/100=11.265 and 754*1.5/100=11.31 both round to 11).
    EXPECT_DOUBLE_EQ(renphobike::autoResistanceFromSlope(754, 0, 0, 0.0, 0.0), 16.0);
}

TEST(RenphoAutoResistanceFromSlopeTest, FlatGroundBaselineIsTunable) {
    // With the defaults a 0% grade lands on resistance 5, which testers reported as far too
    // light on a RENPHO whose physical dial goes to 20 (issue #4873). The rider's configured
    // "Bike Resistance Offset/Gain" must shift that baseline, so this is tunable rather than
    // hardcoded -- renphobike previously ignored those settings entirely.
    EXPECT_DOUBLE_EQ(renphobike::autoResistanceFromSlope(0, 0, 0, 0.0, 0.0), 5.0);
    EXPECT_DOUBLE_EQ(renphobike::autoResistanceFromSlope(0, 0, 0, 0.0, 0.0, 1.0, 10), 11.0);

    // Gain scales how strongly grade swings the resistance around that baseline.
    EXPECT_DOUBLE_EQ(renphobike::autoResistanceFromSlope(400, 0, 0, 0.0, 0.0, 1.0, 4), 11.0);
    EXPECT_DOUBLE_EQ(renphobike::autoResistanceFromSlope(400, 0, 0, 0.0, 0.0, 2.0, 4), 17.0);
}

TEST(RenphoAutoResistanceFromSlopeTest, CRRAndCWGainsAreOffByDefault) {
    // CRRGain/CWGain default to 0 in QZSettings, so an arbitrary crr byte must not perturb the
    // result unless the user has explicitly tuned those gains.
    EXPECT_DOUBLE_EQ(renphobike::autoResistanceFromSlope(751, 200, 200, 0.0, 0.0),
                      renphobike::autoResistanceFromSlope(751, 0, 0, 0.0, 0.0));
}

TEST(RenphoResistanceReconcilerTest, MatchingReadingsNeverReconcile) {
    // Baseline from the real log: grade 751 -> auto resistance 16, gears=8 -> expected 24.
    renphobike::ResistanceReconciler reconciler;
    reconciler.setExpected(16.0 + 8.0);

    EXPECT_DOUBLE_EQ(reconciler.feed(24.0), 0.0);
    EXPECT_DOUBLE_EQ(reconciler.feed(24.0), 0.0);
    EXPECT_DOUBLE_EQ(reconciler.feed(24.0), 0.0);
}

TEST(RenphoResistanceReconcilerTest, SingleGlitchReadingIsIgnored) {
    // A one-sample blip away from the expectation (e.g. a stray BLE notification) must not be
    // confirmed: it needs to repeat before it can be trusted.
    renphobike::ResistanceReconciler reconciler;
    reconciler.setExpected(24.0);

    EXPECT_DOUBLE_EQ(reconciler.feed(24.0), 0.0);
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 0.0); // unconfirmed, first sample of a new value
    EXPECT_DOUBLE_EQ(reconciler.feed(24.0), 0.0); // back to matching before it could confirm
}

TEST(RenphoResistanceReconcilerTest, SustainedDisagreementReconcilesExactly) {
    // The user turns the knob so the bike settles on 30 instead of the expected 24 (grade 751,
    // gears=8). Once that reading repeats, the reconciler must report the exact discrepancy so
    // the caller can realign gears (new gear = old gear + discrepancy = 8 + 6 = 14), matching
    // the requirement that the next slope write (grade unchanged) reproduce the knob's value.
    renphobike::ResistanceReconciler reconciler;
    reconciler.setExpected(16.0 + 8.0);

    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 0.0); // first sample, unconfirmed
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 6.0); // confirmed: 30 - 24 = 6
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 0.0); // already reconciled, must not refire
}

TEST(RenphoResistanceReconcilerTest, SubQuantizationDiscrepancyIsIgnored) {
    // Resistance is reported in 0.5 steps; a discrepancy smaller than that is noise, not a
    // real knob move.
    renphobike::ResistanceReconciler reconciler;
    reconciler.setExpected(24.0);

    EXPECT_DOUBLE_EQ(reconciler.feed(24.3), 0.0);
    EXPECT_DOUBLE_EQ(reconciler.feed(24.3), 0.0);
}

TEST(RenphoResistanceReconcilerTest, NewExpectationResetsDebounceBeforeReconciling) {
    // Simulates a new ROUVY grade arriving right after a reconciled knob move: the bike needs
    // a sample or two to settle onto the new target, so a fresh expectation must not
    // immediately reconcile again even if the very next reading still disagrees.
    renphobike::ResistanceReconciler reconciler;
    reconciler.setExpected(24.0);
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 0.0);
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 6.0); // reconciled once

    reconciler.setExpected(30.0); // new grade recomputed with the just-reconciled gear
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 0.0); // matches immediately, no reconciliation needed

    reconciler.setExpected(20.0); // a further, unrelated grade change
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 0.0); // first sample against the new expectation
    EXPECT_DOUBLE_EQ(reconciler.feed(30.0), 10.0); // confirmed against the new baseline
}
