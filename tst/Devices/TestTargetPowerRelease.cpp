#include <gtest/gtest.h>

#include "Tools/testsettings.h"
#include "characteristics/characteristicnotifier0002.h"
#include "characteristics/characteristicnotifier0004.h"
#include "characteristics/characteristicwriteprocessor0003.h"
#include "characteristics/characteristicwriteprocessor2ad9.h"
#include "characteristics/characteristicwriteprocessore005.h"
#include "devices/bike.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualbike.h"

// Issue 5316: on a bike that opts in (FitPlus with the Virtufit Etappe layout), the app's resistance is kept
// but not applied while the T.Power(W) tile re-sends its target, and "Return to app" restores it at once.
// FTMS over BLE and over DIRCON both reach the bike through CharacteristicWriteProcessor2AD9::writeProcess();
// the iOS BLE bridge goes through virtualbike::applyIosAppCommands().

namespace {
class ReleaseTestBike : public bike {
  public:
    bool targetPowerReleaseSupported() override { return supported; }
    resistance_t resistanceFromPowerRequest(uint16_t power) override { return power / 10; }
    resistance_t rawResistance() const { return lastRawRequestedResistanceValue; }
    bool supported = true;
};

const char *kGrade5 = "110000f4012833";  // simulation parameters, grade 5.00 %
const char *kGrade10 = "110000e8032833"; // grade 10.00 %
const char *kLevel5 = "0432";            // target resistance level 5.0
const char *kPower200 = "05c800";
const char *kPower150 = "059600";

class TargetPowerReleaseTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("TargetPowerRelease")};
    ReleaseTestBike device;
    CharacteristicWriteProcessor2AD9 processor{1, 0, &device, nullptr};
    QByteArray lastReply;

    void SetUp() override {
        settings.activate();
        settings.qsettings.clear();
    }

    void write(const char *hex) {
        lastReply.clear();
        ASSERT_EQ(processor.writeProcess(0x2AD9, QByteArray::fromHex(hex), lastReply), CP_OK);
    }

    // T.Power(W) "+": homeform::Plus() sets zwift_erg and the target, homeform::update() starts the loop
    void tilePlus(int32_t power) {
        settings.qsettings.setValue(QZSettings::zwift_erg, true);
        device.changePower(power);
        device.setTargetPowerOverrideActive(true);
    }

    bool ergMode() { return settings.qsettings.value(QZSettings::zwift_erg, false).toBool(); }

    // what the same command does with no override, on a fresh bike
    resistance_t referenceResistance(const char *hex) {
        ReleaseTestBike reference;
        CharacteristicWriteProcessor2AD9 referenceProcessor(1, 0, &reference, nullptr);
        QByteArray reply;
        referenceProcessor.writeProcess(0x2AD9, QByteArray::fromHex(hex), reply);
        return reference.rawResistance();
    }

    // one bikeProvider() tick of the iOS bridge: the frame sets zwift_erg as bikeProvider() does
    virtualbike::IosAppCommands iosApplied;
    void iosTick(const char *hex, double slope, double crr, double cw, double power, qint64 lastFrame, qint64 now) {
        const QByteArray frame = QByteArray::fromHex(hex);
        if (!frame.isEmpty()) {
            if (frame.at(0) == FTMS_SET_TARGET_POWER) {
                settings.qsettings.setValue(QZSettings::zwift_erg, true);
            } else if (frame.at(0) == FTMS_SET_TARGET_INCLINATION || frame.at(0) == FTMS_SET_TARGET_RESISTANCE_LEVEL ||
                       frame.at(0) == FTMS_SET_INDOOR_BIKE_SIMULATION_PARAMS) {
                settings.qsettings.setValue(QZSettings::zwift_erg, false);
            }
        }
        virtualbike::applyIosAppCommands(&processor, &device, iosApplied, frame, slope, crr, cw, power, lastFrame,
                                         now, ergMode());
    }
};
} // namespace

TEST_F(TargetPowerReleaseTest, AppGradeIsKeptButNotAppliedWhileTileOverrides) {
    write(kGrade5);
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade5));

    tilePlus(250);
    EXPECT_EQ(device.rawResistance(), 25);

    write(kGrade10);
    EXPECT_EQ(device.rawResistance(), 25);
    EXPECT_EQ(lastReply, QByteArray::fromHex("801101")); // the app still gets its success reply
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 250);
}

TEST_F(TargetPowerReleaseTest, ReturnToAppRestoresLatestGradeAtOnce) {
    write(kGrade5);
    tilePlus(250);
    write(kGrade5);
    write(kGrade10);

    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_FALSE(ergMode()); // bikeProvider() stops re-sending the tile's target
    EXPECT_FALSE(device.targetPowerReleaseAvailable());
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade10));

    // the next grade applies again
    write(kGrade5);
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade5));
}

TEST_F(TargetPowerReleaseTest, ReturnToAppRestoresResistanceLevel) {
    tilePlus(250);
    write(kLevel5);
    EXPECT_EQ(device.rawResistance(), 25);

    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_EQ(device.rawResistance(), 5);
}

TEST_F(TargetPowerReleaseTest, AppErgTargetStillApplies) {
    tilePlus(250);
    write(kPower200);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 200);
    EXPECT_EQ(device.rawResistance(), 20);
    EXPECT_TRUE(ergMode());
}

TEST_F(TargetPowerReleaseTest, ErgFrameReplacesStoredGrade) {
    write(kGrade5);
    write(kPower150);
    tilePlus(300);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 300);

    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 150);
    EXPECT_EQ(device.rawResistance(), 15);
    EXPECT_TRUE(ergMode());
}

TEST_F(TargetPowerReleaseTest, TileTargetResentByBikeProviderIsNotAnAppCommand) {
    write(kGrade5);
    tilePlus(250);
    processor.changePower(250); // bikeProvider(): no app frame for 2 s in ERG

    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_FALSE(ergMode());
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade5));
}

TEST_F(TargetPowerReleaseTest, QzResistanceStillApplies) {
    // a QZ training program and the Resistance tile call changeResistance() directly
    write(kGrade5);
    tilePlus(250);
    device.changeResistance(7);
    EXPECT_EQ(device.rawResistance(), 7);
}

TEST_F(TargetPowerReleaseTest, BikeWithoutOptInBehavesAsBefore) {
    device.supported = false;
    tilePlus(250);
    write(kGrade5);
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade5));
    EXPECT_FALSE(device.targetPowerReleaseAvailable());
}

TEST_F(TargetPowerReleaseTest, ButtonNeedsOverrideAppCommandAndAutoResistance) {
    tilePlus(250);
    EXPECT_FALSE(device.targetPowerReleaseAvailable()); // no app command to give back

    write(kGrade5);
    EXPECT_TRUE(device.targetPowerReleaseAvailable());

    device.setAutoResistance(false);
    EXPECT_FALSE(device.targetPowerReleaseAvailable());
    device.setAutoResistance(true);

    device.setTargetPowerOverrideActive(false); // Start, Pause, Stop
    EXPECT_FALSE(device.targetPowerReleaseAvailable());
}

TEST_F(TargetPowerReleaseTest, AutoResistanceOffRecordsAppResistanceAsBefore) {
    // changeResistance() then only records the request (Target Resistance tile), the bike isn't driven
    device.setAutoResistance(false);
    tilePlus(250);
    write(kGrade5);
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade5));
}

TEST_F(TargetPowerReleaseTest, StopEndsOverrideWithoutRestoring) {
    write(kGrade5);
    tilePlus(250);
    write(kGrade10);

    device.setTargetPowerOverrideActive(false);
    EXPECT_EQ(device.rawResistance(), 25); // the load stays until the app's next command

    write(kGrade10);
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade10));
}

TEST_F(TargetPowerReleaseTest, ReleaseWithoutAppCommandChangesNothing) {
    tilePlus(250);
    EXPECT_FALSE(device.releaseTargetPowerToApp());
    EXPECT_EQ(device.rawResistance(), 25);
}

TEST_F(TargetPowerReleaseTest, WahooGradeAndErgOverDircon) {
    CharacteristicWriteProcessorE005 wahoo(1, 0, &device);
    QByteArray reply;
    // setSimMode first: the grade uses its rolling and wind resistance
    const QByteArray simMode = QByteArray::fromHex("43000028003300");
    // setSimGrade 5 %: ((0.05 + 1) / 2) * 65535 = 0x8667
    const QByteArray grade5 = QByteArray::fromHex("466786");
    ASSERT_EQ(wahoo.writeProcess(0xE005, simMode, reply), CP_OK);

    ReleaseTestBike reference;
    CharacteristicWriteProcessorE005 referenceWahoo(1, 0, &reference);
    referenceWahoo.writeProcess(0xE005, simMode, reply);
    referenceWahoo.writeProcess(0xE005, grade5, reply);

    tilePlus(250);
    ASSERT_EQ(wahoo.writeProcess(0xE005, grade5, reply), CP_OK);
    EXPECT_EQ(device.rawResistance(), 25);
    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_EQ(device.rawResistance(), reference.rawResistance());

    tilePlus(250);
    ASSERT_EQ(wahoo.writeProcess(0xE005, QByteArray::fromHex("42c800"), reply), CP_OK); // setErgMode 200 W
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 200);
    tilePlus(300);
    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 200);
}

TEST_F(TargetPowerReleaseTest, ZwiftHubGradeAndErg) {
    CharacteristicNotifier0002 notifier0002(&device);
    CharacteristicNotifier0004 notifier0004(&device);
    CharacteristicWriteProcessor0003 hub(1, 0, &device, &notifier0002, &notifier0004);
    QByteArray reply;
    // slope change: field 0x22, length 3, inner 0x10, zigzag varint of 500 (5.00 %) = 1000 = e8 07
    const QByteArray grade5 = QByteArray::fromHex("04220310e807");
    // power request: 0x18, varint 200 = c8 01
    const QByteArray power200 = QByteArray::fromHex("0418c801");

    ReleaseTestBike reference;
    CharacteristicNotifier0002 referenceNotifier0002(&reference);
    CharacteristicNotifier0004 referenceNotifier0004(&reference);
    CharacteristicWriteProcessor0003 referenceHub(1, 0, &reference, &referenceNotifier0002, &referenceNotifier0004);
    referenceHub.writeProcess(0x0003, grade5, reply);
    ASSERT_GT(reference.rawResistance(), 0);

    tilePlus(250);
    ASSERT_EQ(hub.writeProcess(0x0003, grade5, reply), CP_OK);
    EXPECT_EQ(device.rawResistance(), 25);
    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_EQ(device.rawResistance(), reference.rawResistance());

    tilePlus(250);
    ASSERT_EQ(hub.writeProcess(0x0003, power200, reply), CP_OK);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 200);
    tilePlus(300);
    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 200);
}

TEST_F(TargetPowerReleaseTest, IosBridgeKeepsTileTargetLikeAndroid) {
    const qint64 now = 100000;
    iosTick(kGrade5, 500, 40, 51, 0, now, now);
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade5));

    // "+" sets zwift_erg; the old bridge then re-sent the app's power (0 here) and the loop stopped
    tilePlus(250);
    iosTick("", 500, 40, 51, 0, now, now + 1000);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 250);
    EXPECT_EQ(device.rawResistance(), 25);

    // no app frame for 2 s: the bike's own target is re-sent, as on FTMS BLE and DIRCON
    iosTick("", 500, 40, 51, 0, now, now + 3000);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 250);
}

TEST_F(TargetPowerReleaseTest, IosBridgeKeepsAppGradeAndReturnRestoresIt) {
    const qint64 now = 100000;
    iosTick(kGrade5, 500, 40, 51, 0, now, now);
    tilePlus(250);

    iosTick(kGrade10, 1000, 40, 51, 0, now + 1000, now + 1000);
    EXPECT_EQ(device.rawResistance(), 25);
    EXPECT_FALSE(ergMode());

    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade10));

    // the bridge doesn't re-send the same grade every tick
    device.changeResistance(3);
    iosTick("", 1000, 40, 51, 0, now + 1000, now + 2000);
    EXPECT_EQ(device.rawResistance(), 3);
}

TEST_F(TargetPowerReleaseTest, IosBridgeAppliesErgOnceLikeAndroid) {
    const qint64 now = 100000;
    iosTick(kPower200, 0, 0, 0, 200, now, now);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 200);

    tilePlus(250);
    iosTick("", 0, 0, 0, 200, now, now + 1000);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 250);

    iosTick(kPower150, 0, 0, 0, 150, now + 2000, now + 2000);
    EXPECT_DOUBLE_EQ(device.lastRequestedPower().value(), 150);
}

TEST_F(TargetPowerReleaseTest, IosBridgeAppliesCommandLostBehindAnUnreadFrame) {
    const qint64 now = 100000;
    // Swift kept the earlier "request control" frame; the grade only shows in its latest values
    iosTick("00", 500, 40, 51, 0, now, now);
    EXPECT_EQ(device.rawResistance(), referenceResistance(kGrade5));
}

TEST_F(TargetPowerReleaseTest, IosBridgeResistanceLevel) {
    const qint64 now = 100000;
    tilePlus(250);
    iosTick(kLevel5, 0, 0, 0, 0, now, now);
    EXPECT_EQ(device.rawResistance(), 25);
    EXPECT_TRUE(device.releaseTargetPowerToApp());
    EXPECT_EQ(device.rawResistance(), 5);
}
