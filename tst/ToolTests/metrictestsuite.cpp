#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>
#include <QThread>
#include <cmath>

#include "metric.h"
#include "qzsettings.h"

class MetricMaxWattTest : public testing::Test {
protected:
    void SetUp() override {
        originalOrganization = QCoreApplication::organizationName();
        originalApplication = QCoreApplication::applicationName();
        QCoreApplication::setOrganizationName(QStringLiteral("QDomyosZwiftMetricTests"));
        QCoreApplication::setApplicationName(QStringLiteral("MaxWatt"));

        QSettings settings;
        settings.clear();
        settings.sync();
    }

    void TearDown() override {
        QSettings settings;
        settings.clear();
        settings.sync();
        QCoreApplication::setOrganizationName(originalOrganization);
        QCoreApplication::setApplicationName(originalApplication);
    }

    QString originalOrganization;
    QString originalApplication;
};

TEST_F(MetricMaxWattTest, DefaultMaxWattCapsOutput) {
    metric watts;
    watts.setType(metric::METRIC_WATT, BIKE);

    watts.setValue(10000.0);

    EXPECT_DOUBLE_EQ(watts.value(), QZSettings::default_watt_max);
}

TEST_F(MetricMaxWattTest, ConfiguredMaxWattIsAppliedAfterGainAndOffset) {
    QSettings settings;
    settings.setValue(QZSettings::watt_gain, 2.0);
    settings.setValue(QZSettings::watt_offset, 100.0);
    settings.setValue(QZSettings::watt_max, 900.0);

    metric watts;
    watts.setType(metric::METRIC_WATT, BIKE);
    watts.setValue(500.0);

    EXPECT_DOUBLE_EQ(watts.value(), 900.0);
}

TEST_F(MetricMaxWattTest, ZeroMaxWattDisablesCap) {
    QSettings settings;
    settings.setValue(QZSettings::watt_max, 0.0);

    metric watts;
    watts.setType(metric::METRIC_WATT, BIKE);
    watts.setValue(20000.0);

    EXPECT_DOUBLE_EQ(watts.value(), 20000.0);
}

TEST(MetricMaxWattHelperTest, NonPositiveMaxWattLeavesValueUnchanged) {
    EXPECT_DOUBLE_EQ(metric::capWatt(2000.0, 0.0), 2000.0);
    EXPECT_DOUBLE_EQ(metric::capWatt(2000.0, -1.0), 2000.0);
}

TEST(MetricUpdateSequenceTest, CountsRepeatedHeartRateWrites) {
    metric heart;
    EXPECT_EQ(heart.updateSequence(), 0u);

    heart = 103.0;
    const quint64 healthKitWrite = heart.updateSequence();
    EXPECT_EQ(healthKitWrite, 1u);

    // A device reporting the same BPM must still count as a new source write.
    heart = 103.0;
    EXPECT_EQ(heart.updateSequence(), healthKitWrite + 1);
}

TEST(MetricNonFiniteTest, NanIsShownButKeptOutOfStats) {
    metric resistance;
    resistance.setValue(10.0);
    resistance.setValue(std::nan(""));
    EXPECT_TRUE(std::isnan(resistance.value()));
    resistance.setValue(20.0);

    EXPECT_DOUBLE_EQ(resistance.value(), 20.0);
    EXPECT_DOUBLE_EQ(resistance.average(), 15.0);
    EXPECT_DOUBLE_EQ(resistance.lapAverage(), 15.0);
    EXPECT_DOUBLE_EQ(resistance.min(), 10.0);
    EXPECT_DOUBLE_EQ(resistance.max(), 20.0);
}

TEST(MetricNonFiniteTest, RateAfterNanStaysFinite) {
    metric distance;
    distance.setValue(10.0);
    distance.setValue(20.0);
    distance.setValue(std::nan(""));
    // a real time step, otherwise the rate is 0 whatever the difference is
    QThread::msleep(5);
    distance.setValue(30.0);

    EXPECT_TRUE(std::isfinite(distance.rate1s()));
}

TEST(MetricNonFiniteTest, InfinitiesAreKeptOutOfStats) {
    metric speed;
    speed.setValue(5.0, false);
    speed.setValue(INFINITY, false);
    speed.setValue(-INFINITY, false);
    speed.setValue(7.0, false);

    EXPECT_DOUBLE_EQ(speed.average(), 6.0);
    EXPECT_DOUBLE_EQ(speed.min(), 5.0);
    EXPECT_DOUBLE_EQ(speed.max(), 7.0);
}
