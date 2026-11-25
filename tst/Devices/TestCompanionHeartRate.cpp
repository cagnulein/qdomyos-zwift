#include <gtest/gtest.h>
#include "companionheartrate.h"
#include "qzsettings.h"

TEST(CompanionHeartRate, SelectedBleSensorCannotBeOverwrittenByPhone) {
    // Anonymized sequence from the 2026-11-25 Android debug log:
    // the selected BLE belt reports 85 bpm, then the nearby phone reports 92.
    double currentHeart = 85;
    int deliveries = 0;
    auto receive = [&](double hr) { currentHeart = hr; ++deliveries; };
    const QString selected = QStringLiteral("Selected BLE belt");
    dispatchCompanionHeartRate(QStringLiteral("SENDER=PHONE#HR=92#BCAD=87.0#SPD=22.3#"), selected, receive);
    EXPECT_EQ(currentHeart, 85);
    dispatchCompanionHeartRate(QStringLiteral("SENDER=PHONE#HR=0#"), selected, receive);
    EXPECT_EQ(currentHeart, 85);
    EXPECT_EQ(deliveries, 0);
}

TEST(CompanionHeartRate, SelectedSensorWinsEvenBeforeFirstBleReading) {
    double currentHeart = 0;
    dispatchCompanionHeartRate(QStringLiteral("SENDER=PHONE#HR=92#"),
                              QStringLiteral("Selected BLE belt"),
                              [&](double hr) { currentHeart = hr; });
    EXPECT_EQ(currentHeart, 0);
}

TEST(CompanionHeartRate, NoSelectedBeltPreservesCompanionDelivery) {
    for (const QString &beltName : {QZSettings::default_heart_rate_belt_name, QString(), QStringLiteral("Disabled (none)")}) {
        double currentHeart = 0;
        int deliveries = 0;
        auto receive = [&](double hr) { currentHeart = hr; ++deliveries; };
        dispatchCompanionHeartRate(QStringLiteral("SENDER=PHONE#HR=92#BCAD=87.0#"), beltName, receive);
        EXPECT_EQ(currentHeart, 92);
        dispatchCompanionHeartRate(QStringLiteral("SENDER=PHONE#HR=0#"), beltName, receive);
        EXPECT_EQ(currentHeart, 0);
        EXPECT_EQ(deliveries, 2);
        dispatchCompanionHeartRate(QStringLiteral("SENDER=PHONE#BCAD=87.0#"), beltName, receive);
        EXPECT_EQ(deliveries, 2);
    }
}

TEST(CompanionHeartRate, ChangingSelectionTakesEffectOnNextPacket) {
    double currentHeart = 85;
    auto receive = [&](double hr) { currentHeart = hr; };
    const QString packet = QStringLiteral("SENDER=PHONE#HR=92#");
    dispatchCompanionHeartRate(packet, QStringLiteral("Selected BLE belt"), receive);
    EXPECT_EQ(currentHeart, 85);
    dispatchCompanionHeartRate(packet, QZSettings::default_heart_rate_belt_name, receive);
    EXPECT_EQ(currentHeart, 92);
}
