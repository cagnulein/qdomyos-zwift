#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "characteristics/characteristicnotifier0002.h"
#include "characteristics/characteristicnotifier0004.h"
#include "characteristics/characteristicwriteprocessor0003.h"
#include "devices/bike.h"

// While paused, update_metrics() zeroes m_watt but drivers that read the power from the device packets write it
// back on every packet. The virtual devices read wattsMetricforUI(): it has to give a steady 0 while paused,
// or the real power when instant_power_on_pause is on.

namespace {
class PausePowerTestBike : public bike {
  public:
    void setPacketPower(double watts) { m_watt = watts; } // what a driver does on each device packet
};

class PausePowerTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("PausePower")};
    PausePowerTestBike device;

    void SetUp() override {
        settings.activate();
        settings.qsettings.clear();
    }
};
} // namespace

TEST_F(PausePowerTest, RunningGivesThePower) {
    device.setPacketPower(150);
    EXPECT_DOUBLE_EQ(device.wattsMetricforUI(), 150);
}

TEST_F(PausePowerTest, PausedGivesZeroEvenAfterAPacket) {
    device.setPaused(true);
    device.setPacketPower(150);
    EXPECT_DOUBLE_EQ(device.wattsMetricforUI(), 0);
}

TEST_F(PausePowerTest, PausedWithInstantPowerGivesThePower) {
    settings.qsettings.setValue(QZSettings::instant_power_on_pause, true);
    device.setPaused(true);
    device.setPacketPower(150);
    EXPECT_DOUBLE_EQ(device.wattsMetricforUI(), 150);
}

TEST_F(PausePowerTest, PowerIsBackAfterThePause) {
    device.setPaused(true);
    device.setPacketPower(150);
    device.setPaused(false);
    device.setPacketPower(160);
    EXPECT_DOUBLE_EQ(device.wattsMetricforUI(), 160);
}

TEST_F(PausePowerTest, ZwiftHubRidingDataIsZeroWhilePaused) {
    CharacteristicNotifier0002 notifier0002(&device);
    CharacteristicNotifier0004 notifier0004(&device);
    CharacteristicWriteProcessor0003 hub(1, 0, &device, &notifier0002, &notifier0004);
    // The packet ends with a counter shared by all hubs (calculateUnknown1(), field 0x28): only the fields
    // before it are compared. Type 0x03, then power (0x08), cadence (0x10), speed (0x18) and the power again (0x20,
    // as buildCurrentHubRidingData() fills it), each a varint
    const QByteArray zeroPowerFields = QByteArray::fromHex("030800100018002000");
    const QByteArray power150Fields = QByteArray::fromHex("0308960110001800209601"); // varint 150 = 96 01

    device.setPacketPower(150);
    EXPECT_TRUE(hub.buildCurrentHubRidingData().startsWith(power150Fields));

    device.setPaused(true);
    device.setPacketPower(150);
    EXPECT_TRUE(hub.buildCurrentHubRidingData().startsWith(zeroPowerFields));
}
