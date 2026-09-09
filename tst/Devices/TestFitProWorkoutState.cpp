#include <gtest/gtest.h>

#include "devices/nordictrackifitadbtreadmill/fitproworkoutstate.h"

namespace {

using InitialState = FitProInitialWorkoutState;

TEST(FitProWorkoutStateTest, InitialStoppedStateIsHandledOnlyForFirstSample) {
    EXPECT_EQ(fitproInitialWorkoutState(0, 1), InitialState::Stopped);
    EXPECT_EQ(fitproInitialWorkoutState(0, 5), InitialState::Stopped);
    EXPECT_EQ(fitproInitialWorkoutState(1, 1), InitialState::None);
}

TEST(FitProWorkoutStateTest, InitialPausedStateIsPreserved) {
    EXPECT_EQ(fitproInitialWorkoutState(0, 4), InitialState::Paused);
    EXPECT_EQ(fitproInitialWorkoutState(3, 4), InitialState::None);
}

TEST(FitProWorkoutStateTest, InitialRunningStateDoesNotBecomeAStop) {
    EXPECT_EQ(fitproInitialWorkoutState(0, 3), InitialState::None);
}

}  // namespace
