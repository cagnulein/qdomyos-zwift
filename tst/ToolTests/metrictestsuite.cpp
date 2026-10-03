#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>

#include "metric.h"
#include "qzsettings.h"
#include "devices/cscbike/cscbike.h"

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

TEST(CscBikeWheelSpeedTest, ConvertsWheelRevolutionsToKilometresPerHour) {
    EXPECT_NEAR(cscbike::speedFromWheelRevolutions(100, 101, 1000, 2024, 2070.0), 7.452, 0.000001);
}

TEST(CscBikeWheelSpeedTest, HandlesWheelAndEventTimeCounterWraparound) {
    const double wrapped = cscbike::speedFromWheelRevolutions(0xffffffffu, 1u, 65530u, 10u, 2070.0);
    const double nonWrapped = cscbike::speedFromWheelRevolutions(0u, 2u, 0u, 16u, 2070.0);

    EXPECT_DOUBLE_EQ(wrapped, nonWrapped);
}

TEST(CscBikeWheelSpeedTest, ZeroEventTimeOrCircumferenceProducesZeroSpeed) {
    EXPECT_DOUBLE_EQ(cscbike::speedFromWheelRevolutions(10, 11, 100, 100, 2070.0), 0.0);
    EXPECT_DOUBLE_EQ(cscbike::speedFromWheelRevolutions(10, 11, 100, 200, 0.0), 0.0);
}

class CscBikeSpeedPowerTest : public testing::Test {
protected:
    void SetUp() override {
        originalOrganization = QCoreApplication::organizationName();
        originalApplication = QCoreApplication::applicationName();
        QCoreApplication::setOrganizationName(QStringLiteral("QDomyosZwiftMetricTests"));
        QCoreApplication::setApplicationName(QStringLiteral("CscBikeSpeedPower"));

        QSettings settings;
        settings.clear();
        settings.setValue(QZSettings::weight, 75.0);
        settings.setValue(QZSettings::bike_weight, 10.0);
        settings.setValue(QZSettings::rolling_resistance, 0.005);
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

TEST_F(CscBikeSpeedPowerTest, EstimatedPowerIsZeroWhenStoppedAndIncreasesWithSpeed) {
    const double stoppedPower = metric::calculatePowerFromSpeed(0.0, 0.0);
    const double tenKphPower = metric::calculatePowerFromSpeed(10.0, 0.0);
    const double twentyKphPower = metric::calculatePowerFromSpeed(20.0, 0.0);

    EXPECT_DOUBLE_EQ(stoppedPower, 0.0);
    EXPECT_GT(tenKphPower, stoppedPower);
    EXPECT_GT(twentyKphPower, tenKphPower);
}

TEST_F(CscBikeSpeedPowerTest, SpeedSensorPowerIgnoresReceivedInclination) {
    const double speed = 30.0;
    const double flatRoadPower = cscbike::speedSensorPowerFromSpeed(speed);

    EXPECT_DOUBLE_EQ(flatRoadPower, metric::calculatePowerFromSpeed(speed, 0.0));
    EXPECT_NE(metric::calculatePowerFromSpeed(speed, 10.0), flatRoadPower);
}
