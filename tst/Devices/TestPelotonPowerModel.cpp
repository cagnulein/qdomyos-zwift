#include <gtest/gtest.h>
#include "devices/pelotonbike/pelotonpowermodel.h"

TEST(PelotonBikeErgPowerModel, RolandWettbergTargetsUseCadenceAwareLevels) {
    PelotonPowerModel model;

    // The legacy bike::resistanceFromPowerRequest() used watts / 10:
    // 130 W -> 13, 160 W -> 16, far below the Bike+ power curve.
    EXPECT_EQ(47, model.resistanceFor(130.0f, 70.0f));
    EXPECT_EQ(51, model.resistanceFor(160.0f, 70.0f));
    EXPECT_EQ(40, model.resistanceFor(130.0f, 90.0f));
    EXPECT_EQ(43, model.resistanceFor(160.0f, 90.0f));
}

TEST(PelotonBikeErgPowerModel, RefusesToPredictWithoutValidPedaling) {
    PelotonPowerModel model;
    EXPECT_EQ(-1, model.resistanceFor(130.0f, 0.0f));
    EXPECT_EQ(-1, model.resistanceFor(130.0f, 49.0f));
    EXPECT_EQ(-1, model.resistanceFor(0.0f, 90.0f));
    EXPECT_LE(model.resistanceFor(1000.0f, 90.0f), 75);
}

TEST(PelotonBikeErgPowerModel, LearnsOnlyPlausiblePerBikeSteadyReadings) {
    PelotonPowerModel model;
    const float initial = model.predictedWatts(40, 90.0f);

    model.learn(40, initial * 1.10f, 90.0f);
    EXPECT_NEAR(initial * 1.10f, model.predictedWatts(40, 90.0f), 0.1f);
    // Nearby levels inherit the measured calibration factor.
    EXPECT_NEAR(model.predictedWatts(41, 90.0f),
                PelotonPowerModel::seedWattsAt90(41) * 1.10f, 0.2f);

    model.learn(40, initial * 4.0f, 90.0f); // Outlier must be rejected.
    model.learn(40, initial * 0.4f, 90.0f);
    model.learn(40, initial * 1.2f, 30.0f); // No learning below 50 rpm.
    EXPECT_NEAR(initial * 1.10f, model.predictedWatts(40, 90.0f), 0.1f);

    model.learn(40, initial * 1.20f, 90.0f);
    EXPECT_NEAR(initial * 1.12f, model.predictedWatts(40, 90.0f), 0.1f);
}
