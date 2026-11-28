#include <gtest/gtest.h>

#include "devices/bike.h"
#include "devices/tacxneo2/tacxneo2.h"

// Preserve the existing FTMS delta semantics: only small errors are corrected.
TEST(ExternalPowerErg, FtmsLegacyCorrectionWithinFilter) {
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(150, 150, 141, 10), 159);
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(150, 150, 140, 10), 150);
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(150, 150, 135, 10), 150);
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(150, 150, 155, 10), 145);
}

TEST(ExternalPowerErg, NoCorrectionForStoppedOrInvalidSensor) {
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(0, 150, 135, 30), 0);
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(150, 0, 135, 30), 150);
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(150, 150, 0, 30), 150);
    EXPECT_EQ(bike::powerTargetWithExternalSensorDelta(150, 150, 135, 0), 150);
}

TEST(ExternalPowerErg, ThinkXMatchesAssiomaAt150W) {
    // ThinkRider reports 150 W, Assioma reports 135 W: request 165 W from the trainer.
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 0, true, true, 150, 135), 165);
    // When it then reports 165 W and the pedals report 150 W, maintain the same command.
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 0, true, true, 165, 150), 165);
}

TEST(ExternalPowerErg, OtherTacxAndMissingPowerSensorAreUnchanged) {
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 0, false, true, 150, 135), 150);
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 0, true, false, 150, 135), 150);
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 0, true, true, 150, 0), 150);
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 0, true, true, 0, 135), 150);
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 0, true, true, 190, 135), 150);
}

TEST(ExternalPowerErg, CalibrationAppliedOnceAndReleaseIsAlwaysZero) {
    // The existing Tacx gain/offset and bike offset are applied exactly once.
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 2, 10, 5, true, false, 150, 135), 75);
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(150, 1, 0, 10, true, true, 150, 135), 175);
    EXPECT_EQ(tacxneo2::calibratedErgTrainerPower(0, 1, 0, 10, true, true, 150, 135), 0);
}
