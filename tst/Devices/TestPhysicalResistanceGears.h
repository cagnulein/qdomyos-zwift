#pragma once

#include <gtest/gtest.h>

#include "devices/nordictrackifitadbbike/nordictrackifitadbbike.h"

class PhysicalResistanceGearsTestAccess : public nordictrackifitadbbike {
  public:
    using nordictrackifitadbbike::physicalResistanceGearDelta;
};

TEST(PhysicalResistanceGearsTest, IgnoresMissingOrInitialResistance) {
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(0, 5), 0);
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(5, 0), 0);
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(-1, 5), 0);
}

TEST(PhysicalResistanceGearsTest, AcceptsSmallPositiveAndNegativeChanges) {
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(5, 7), 2);
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(7, 5), -2);
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(5, 10), 5);
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(10, 5), -5);
}

TEST(PhysicalResistanceGearsTest, RapidSuccessiveChangesAccumulateOneStepAtATime) {
    const int firstDelta = PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(5, 6);
    const int secondDelta = PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(6, 7);

    EXPECT_EQ(firstDelta + secondDelta, 2);
}

TEST(PhysicalResistanceGearsTest, GearOffsetRaisesLaterNominalResistanceTarget) {
    const int gearOffset = PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(5, 7);

    EXPECT_EQ(5 + gearOffset, 7);
}

TEST(PhysicalResistanceGearsTest, RejectsLargeJumpAsResynchronization) {
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(5, 12), 0);
    EXPECT_EQ(PhysicalResistanceGearsTestAccess::physicalResistanceGearDelta(12, 5), 0);
}
