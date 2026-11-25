#include <gtest/gtest.h>

#include "devices/treadmillcalories.h"
#include "metric.h"

// Profile used throughout: 131.54 kg, 185.42 cm, 49 y, male -> Mifflin-St Jeor 2234.275 kcal/day
namespace {
const double kWeight = 131.54;
const double kHeight = 185.42;
const double kBmr = 10.0 * kWeight + 6.25 * kHeight - 5.0 * 49 + 5.0;

TreadmillCalories::Tick tick(double dt, double speed, double incline, bool useModel, double legacy,
                             bool clockRunning = true) {
    TreadmillCalories::Tick t;
    t.dtSec = dt;
    t.clockRunning = clockRunning;
    t.speedKmh = speed;
    t.inclinePct = incline;
    t.weightKg = kWeight;
    t.heightCm = kHeight;
    t.restingKcalPerDay = kBmr;
    t.useModel = useModel;
    t.legacyKcal = legacy;
    return t;
}

// How treadmill uses it: a metric accumulates what tick() adds, cleared with clear(true) at a
// new session like the other accumulators
struct Session {
    TreadmillCalories calories;
    metric kcal;
    double legacy = 0; // the power-based total the driver keeps

    void start() {
        kcal.clear(true);
        calories.reset(legacy);
    }
    void run(int seconds, double speed, double incline, bool useModel, double legacyPerSec,
             bool clockRunning = true) {
        for (int i = 0; i < seconds; i++) {
            legacy += legacyPerSec;
            kcal += calories.tick(tick(1.0, speed, incline, useModel, legacy, clockRunning));
        }
    }
};
} // namespace

TEST(TreadmillCalories, WalkRunTransitionFollowsHeight) {
    // Froude 0.5 with leg length 0.5 x height: 1.72 m -> 2.05 m/s (Hreljac measured 2.06)
    EXPECT_NEAR(TreadmillCalories::walkRunTransitionKmh(172.0) / 3.6, 2.054, 0.001);
    EXPECT_NEAR(TreadmillCalories::walkRunTransitionKmh(kHeight), 7.68, 0.01);
    // out-of-range heights fall back to 175 cm
    EXPECT_DOUBLE_EQ(TreadmillCalories::walkRunTransitionKmh(0), TreadmillCalories::walkRunTransitionKmh(175.0));
}

TEST(TreadmillCalories, RunningMatchesAcsmByHand) {
    // ACSM net 0.2 S + 0.9 S G at 11 km/h, 2 %: 39.967 mL/kg/min, plus resting
    bool running = false;
    const double perMin = TreadmillCalories::kcalPerMinute(11.0, 2.0, kWeight, kHeight, kBmr, &running);
    EXPECT_TRUE(running);
    const double S = 11.0 * 1000.0 / 60.0;
    const double expected = kBmr / 1440.0 + (0.2 * S + 0.9 * S * 0.02) * kWeight * 20.1 / 4184.0;
    EXPECT_NEAR(perMin, expected, 1e-9);
    EXPECT_NEAR(perMin, 26.81, 0.01);
}

TEST(TreadmillCalories, StoppedBeltGivesTheRestingRate) {
    EXPECT_DOUBLE_EQ(TreadmillCalories::kcalPerMinute(0, 3.0, kWeight, kHeight, kBmr), kBmr / 1440.0);
}

TEST(TreadmillCalories, SettingOffKeepsThePowerBasedTotalExactly) {
    Session s;
    s.start();
    s.run(600, 5.0, 3.0, false, 0.37);
    EXPECT_FALSE(s.calories.modelUsed());
    EXPECT_DOUBLE_EQ(s.calories.shown(s.legacy), s.legacy);
}

TEST(TreadmillCalories, ConsecutiveWorkoutsStartFromZero) {
    Session s;
    s.start();
    s.run(300, 5.0, 3.0, true, 0.0);
    const double first = s.kcal.value();
    EXPECT_GT(first, 0.0);

    s.start(); // second workout
    EXPECT_NEAR(s.kcal.value(), 0.0, 1e-9);
    s.run(300, 5.0, 3.0, true, 0.0);
    EXPECT_NEAR(s.kcal.value(), first, 1e-9);
    EXPECT_NEAR(s.kcal.value(), s.calories.total(), 1e-9);

    s.start(); // and a third
    s.run(300, 5.0, 3.0, true, 0.0);
    EXPECT_NEAR(s.kcal.value(), first, 1e-9);
}

TEST(TreadmillCalories, ModelTotalIsTheSumOfItsTicks) {
    Session s;
    s.start();
    s.run(120, 3.54, 3.0, true, 0.0);
    const double perMin = TreadmillCalories::kcalPerMinute(3.54, 3.0, kWeight, kHeight, kBmr);
    EXPECT_NEAR(s.kcal.value(), perMin * 2.0, 1e-9);
}

TEST(TreadmillCalories, PauseAddsNothingAndResumeContinues) {
    Session s;
    s.start();
    s.run(60, 5.0, 0.0, true, 0.0);
    const double beforePause = s.kcal.value();
    // treadmill does not tick while paused; the driver may still move its own total
    s.legacy += 5.0;
    EXPECT_NEAR(s.kcal.value(), beforePause, 1e-9);
    s.run(60, 5.0, 0.0, true, 0.0);
    EXPECT_NEAR(s.kcal.value(), 2.0 * beforePause, 1e-9);
}

TEST(TreadmillCalories, LapCountsFromTheLap) {
    Session s;
    s.start();
    s.run(60, 5.0, 0.0, true, 0.0);
    const double lap1 = s.kcal.value();
    s.kcal.setLap(true);
    s.run(60, 5.0, 0.0, true, 0.0);
    EXPECT_NEAR(s.kcal.lapValue(), lap1, 1e-9);
    EXPECT_NEAR(s.kcal.value(), 2.0 * lap1, 1e-9);
}

TEST(TreadmillCalories, ConsoleCaloriesAreKept) {
    // a driver that reads the calories from the console: the model is not used
    Session s;
    s.start();
    s.run(300, 5.0, 3.0, false, 0.5);
    EXPECT_DOUBLE_EQ(s.calories.shown(s.legacy), 150.0);
}

TEST(TreadmillCalories, StandaloneStrydKeepsThePowerBasedTotal) {
    // strydrunpowersensor measures the watts from the start: the model is never used
    Session s;
    s.start();
    s.run(600, 10.0, 1.0, false, 0.4);
    EXPECT_FALSE(s.calories.modelUsed());
    EXPECT_DOUBLE_EQ(s.calories.shown(s.legacy), s.legacy);
}

TEST(TreadmillCalories, PowerSensorConnectingMidWorkoutDoesNotJump) {
    Session s;
    s.legacy = 0;
    s.start();
    s.run(300, 5.0, 3.0, true, 0.9); // the power-based total runs higher meanwhile
    const double before = s.calories.shown(s.legacy);
    // sensor connects: from now on only the power-based increases are added
    s.run(1, 5.0, 3.0, false, 0.4);
    EXPECT_NEAR(s.calories.shown(s.legacy), before + 0.4, 1e-9);
    s.run(99, 5.0, 3.0, false, 0.4);
    EXPECT_NEAR(s.calories.shown(s.legacy), before + 40.0, 1e-9);
    EXPECT_NEAR(s.kcal.value(), before + 40.0, 1e-9);
}

TEST(TreadmillCalories, SettingTurnedOnMidWorkoutContinuesFromThePowerBasedTotal) {
    Session s;
    s.start();
    s.run(300, 5.0, 3.0, false, 0.9);
    const double powerBased = s.calories.shown(s.legacy);
    EXPECT_NEAR(powerBased, 270.0, 1e-9);
    s.run(1, 5.0, 3.0, true, 0.9);
    const double perSec = TreadmillCalories::kcalPerMinute(5.0, 3.0, kWeight, kHeight, kBmr) / 60.0;
    EXPECT_NEAR(s.calories.shown(s.legacy), powerBased + perSec, 1e-9);
    EXPECT_NEAR(s.kcal.value(), powerBased + perSec, 1e-9);
}

TEST(TreadmillCalories, SettingTurnedOffMidWorkoutDoesNotJump) {
    Session s;
    s.start();
    s.run(300, 5.0, 3.0, true, 0.9);
    const double model = s.calories.shown(s.legacy);
    s.run(10, 5.0, 3.0, false, 0.9);
    EXPECT_NEAR(s.calories.shown(s.legacy), model + 9.0, 1e-9);
}

TEST(TreadmillCalories, ActiveCaloriesDoNotDropWhileTheClockRunsAtZeroSpeed) {
    // continuous_moving: the clock runs with the belt stopped; resting calories still count, so
    // total minus resting (active) stays where it was
    Session s;
    s.start();
    s.run(60, 5.0, 0.0, true, 0.0);
    const double activeBefore = s.kcal.value() - kBmr / 1440.0 * 1.0;
    s.run(120, 0.0, 0.0, true, 0.0, true);
    const double activeAfter = s.kcal.value() - kBmr / 1440.0 * 3.0;
    EXPECT_NEAR(activeAfter, activeBefore, 1e-9);
}

TEST(TreadmillCalories, NoCaloriesWhileTheClockIsStopped) {
    Session s;
    s.start();
    s.run(60, 5.0, 0.0, true, 0.0);
    const double before = s.kcal.value();
    s.run(60, 0.0, 0.0, true, 0.0, false);
    EXPECT_NEAR(s.kcal.value(), before, 1e-9);
}

TEST(TreadmillCalories, ASourceThatResetsTakesNothingAway) {
    Session s;
    s.start();
    s.run(60, 5.0, 0.0, false, 1.0);
    s.legacy = 0; // console counter restarted
    s.kcal += s.calories.tick(tick(1.0, 5.0, 0.0, false, s.legacy));
    EXPECT_NEAR(s.kcal.value(), 60.0, 1e-9);
}
