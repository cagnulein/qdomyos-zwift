#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>

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
