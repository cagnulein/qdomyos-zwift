#include <gtest/gtest.h>
#include <utility>

#include "Tools/testsettings.h"
#include "characteristics/characteristicnotifier2acd.h"
#include "characteristics/characteristicwriteprocessor2ad9.h"
#include "devices/elliptical.h"
#include "devices/horizontreadmill/ypoom5control.h"
#include "devices/treadmill.h"

namespace {
class ControlPointTestTreadmill : public treadmill {
  public:
    void changeSpeed(double value) override { receivedSpeed = value; }
    void changeInclination(double grade, double) override { receivedIncline = grade; }
    void start() override { ++startCalls; }
    void stop(bool) override { ++stopCalls; }
    void reportIncline(double value) { Inclination = value; }
    double receivedSpeed = -1;
    double receivedIncline = -100;
    unsigned startCalls = 0;
    unsigned stopCalls = 0;
};

class ControlPointTestElliptical : public elliptical {
  public:
    bool inclinationAvailableByHardware() override { return true; }
    void changeInclination(double grade, double) override { receivedIncline = grade; }
    double receivedIncline = -100;
};

class FtmsTreadmillControlTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("FtmsTreadmillControl")};
    ControlPointTestTreadmill treadmill;
    CharacteristicWriteProcessor2AD9 processor{1, 0, &treadmill, nullptr};
    void SetUp() override { settings.activate(); }
};
} // namespace

TEST_F(FtmsTreadmillControlTest, DecodesSpeedTargetsAcrossHighBitByteBoundaries) {
    struct Example {
        const char *packet;
        double speed;
    };
    for (const auto &example : {Example{"027800", 1.2}, Example{"028200", 1.3}, Example{"02c800", 2.0},
                                Example{"020401", 2.6}, Example{"025807", 18.8}}) {
        QByteArray reply;
        ASSERT_EQ(processor.writeProcess(0x2AD9, QByteArray::fromHex(example.packet), reply), CP_OK);
        EXPECT_DOUBLE_EQ(treadmill.receivedSpeed, example.speed) << example.packet;
    }
}

TEST_F(FtmsTreadmillControlTest, DecodesSignedInclineAndHighBitPositiveTargets) {
    struct Example {
        const char *packet;
        double incline;
    };
    for (const auto &example : {Example{"03c4ff", -6}, Example{"03f6ff", -1}, Example{"030000", 0},
                                Example{"038200", 13}, Example{"039600", 15}}) {
        QByteArray reply;
        ASSERT_EQ(processor.writeProcess(0x2AD9, QByteArray::fromHex(example.packet), reply), CP_OK);
        EXPECT_DOUBLE_EQ(treadmill.receivedIncline, example.incline) << example.packet;
    }
}

TEST_F(FtmsTreadmillControlTest, SharedProcessorPreservesStartStopForwardingWithoutStartingOtherModels) {
    QList<QByteArray> forwarded;
    QObject::connect(&processor, &CharacteristicWriteProcessor2AD9::ftmsCharacteristicChanged,
                     [&forwarded](const QLowEnergyCharacteristic &, const QByteArray &data) {
                         forwarded.append(data);
                     });
    for (const char *packet : {"00", "07", "0801"}) {
        const QByteArray data = QByteArray::fromHex(packet);
        QByteArray reply;
        ASSERT_EQ(processor.writeProcess(0x2AD9, data, reply), CP_OK);
        ASSERT_FALSE(forwarded.isEmpty());
        EXPECT_EQ(forwarded.last(), data);
        EXPECT_EQ(reply, QByteArray::fromHex("80") + data.left(1) + QByteArray::fromHex("01"));
    }
    EXPECT_EQ(forwarded.size(), 3);
    EXPECT_EQ(treadmill.startCalls, 0u);
    EXPECT_EQ(treadmill.stopCalls, 0u);
}

TEST_F(FtmsTreadmillControlTest, SharedDecoderPreservesEllipticalInclineRouting) {
    ControlPointTestElliptical elliptical;
    CharacteristicWriteProcessor2AD9 ellipticalProcessor(1, 0, &elliptical, nullptr);
    for (const auto &example : {std::pair<const char *, double>{"030a00", 1},
                                {"038200", 13}, {"03c4ff", -6}}) {
        QByteArray reply;
        ASSERT_EQ(ellipticalProcessor.writeProcess(0x2AD9, QByteArray::fromHex(example.first), reply), CP_OK);
        EXPECT_DOUBLE_EQ(elliptical.receivedIncline, example.second);
    }
}

TEST_F(FtmsTreadmillControlTest, PreservesDeclineInVirtualTreadmillTelemetry) {
    settings.qsettings.setValue(QZSettings::bike_cadence_sensor, false);
    settings.qsettings.setValue(QZSettings::real_inclination_to_virtual_treamill_bridge, false);
    CharacteristicNotifier2ACD notifier(&treadmill);
    for (double incline : {-6.0, -1.0, 0.0, 1.0, 15.0}) {
        treadmill.reportIncline(incline);
        QByteArray packet;
        ASSERT_EQ(notifier.notify(packet), CN_OK);
        double reportedSpeed = 0;
        double reportedIncline = 0;
        ASSERT_TRUE(YpooM5Control::readTelemetry(reinterpret_cast<const uint8_t *>(packet.constData()),
                                                packet.size(), reportedSpeed, reportedIncline));
        EXPECT_DOUBLE_EQ(reportedIncline, incline);
    }
}
