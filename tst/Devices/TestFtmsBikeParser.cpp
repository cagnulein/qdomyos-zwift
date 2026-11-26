#include "TestFtmsBikeParser.h"

#include "devices/ftmsbike/ftmsbike.h"
#include "devices/daumbike/daumbike.h"
#include "Tools/testsettings.h"
#include "characteristics/characteristicwriteprocessor2ad9.h"
#include <QBluetoothAddress>
#include <QMetaObject>

TEST(FtmsBikeParserTest, DecodesFsIb50Resistance) {
    resistance_t resistance = -1;
    const QByteArray packet = QByteArray::fromHex("02 42 02 47 07 1e 34 00 00 00 00 00 00 00 00");

    ASSERT_TRUE(ftmsbike::parseFsIb50Resistance(packet, &resistance));
    EXPECT_EQ(30, resistance);
}

TEST(FtmsBikeParserTest, DecodesMinimumAndMaximumObservedLevels) {
    resistance_t resistance = -1;

    EXPECT_TRUE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 d6 07 01 36 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_EQ(1, resistance);

    EXPECT_TRUE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 41 08 20 3b 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_EQ(32, resistance);
}

TEST(FtmsBikeParserTest, RejectsNonFsIb50Packets) {
    resistance_t resistance = -1;

    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 47 07 1e 34 00 00 00 00 00 00 00"), &resistance));
    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 44 05 41 03 00 00 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 47 07 21 34 00 00 00 00 00 00 00 00"), &resistance));
    EXPECT_FALSE(ftmsbike::parseFsIb50Resistance(
        QByteArray::fromHex("02 42 02 47 07 1e 34 00 00 00 00 00 00 00 00"), nullptr));
}

TEST(FtmsBikeParserTest, IgnoresResistanceChangesDuringInclinationHysteresis) {
    resistance_t delta = 0;

    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(14, 24, 1999, -1, &delta));
    EXPECT_EQ(0, delta);
}

TEST(FtmsBikeParserTest, ConvertsResistanceChangesOutsideInclinationHysteresisToGearDelta) {
    resistance_t delta = 0;

    EXPECT_TRUE(ftmsbike::fsIb50ResistanceGearDelta(14, 24, 2000, -1, &delta));
    EXPECT_EQ(10, delta);
    EXPECT_TRUE(ftmsbike::fsIb50ResistanceGearDelta(24, 14, -1, -1, &delta));
    EXPECT_EQ(-10, delta);
}

TEST(FtmsBikeParserTest, DoesNotCreateGearDeltaFromAnUninitializedOrUnchangedBaseline) {
    resistance_t delta = 0;

    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(-1, 14, -1, -1, &delta));
    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(14, 14, -1, -1, &delta));
    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(14, 24, 2000, -1, nullptr));
}

TEST(FtmsBikeParserTest, IgnoresResistanceChangesWhileInclinationMetricIsActive) {
    resistance_t delta = 0;

    EXPECT_FALSE(ftmsbike::fsIb50ResistanceGearDelta(1, 3, 3553, 500, &delta));
    EXPECT_EQ(0, delta);
}

TEST(FtmsBikeParserTest, AppliesGearGainToFsIb50PhysicalGearDelta) {
    EXPECT_DOUBLE_EQ(0.05, ftmsbike::applyFsIb50GearGain(1, 0.05));
    EXPECT_DOUBLE_EQ(-0.2, ftmsbike::applyFsIb50GearGain(-4, 0.05));
}

TEST(FtmsBikeParserTest, KeepsFullFsIb50GearDeltaWithDefaultGain) {
    EXPECT_DOUBLE_EQ(5.0, ftmsbike::applyFsIb50GearGain(5, 1.0));
}

TEST(DaumBikeControlTest, AllowsZeroPowerFallbackWithConnectedVirtualFtmsBike) {
    EXPECT_TRUE(daumbike::shouldWriteResistanceToDaum(true, true, 0));
}

TEST(DaumBikeControlTest, KeepsOrdinaryPendingResistanceBlockedWithConnectedVirtualFtmsBike) {
    EXPECT_FALSE(daumbike::shouldWriteResistanceToDaum(true, true, -1));
}

TEST(DaumBikeControlTest, AllowsDaumResistanceWithoutVirtualFtmsOwnership) {
    EXPECT_TRUE(daumbike::shouldWriteResistanceToDaum(false, false, -1));
    EXPECT_TRUE(daumbike::shouldWriteResistanceToDaum(true, false, -1));
}


namespace {
class BodytoneSlopeTestBike : public ftmsbike {
  public:
    BodytoneSlopeTestBike() : ftmsbike(false, false, 4, 1.0) {}
    void identify(const QString &name) {
        bluetoothDevice = QBluetoothDeviceInfo(QBluetoothAddress(QStringLiteral("C4:33:36:33:50:3F")), name, 0);
        Speed = 20.0;
        Cadence = 75.0;
    }
    bool receive(const QByteArray &packet) {
        return QMetaObject::invokeMethod(this, "ftmsCharacteristicChanged", Qt::DirectConnection,
                                        Q_ARG(QLowEnergyCharacteristic, QLowEnergyCharacteristic()),
                                        Q_ARG(QByteArray, packet));
    }
    bool slopeActive() const { return m_slopeControlEnabled; }
    int pendingPower() const { return requestPower; }
    void refreshSlope() { updateSlopeTargetPower(true); }
    void speedForTest(double speed) { Speed = speed; }
};

class BodytoneSlopePowerTest : public ::testing::Test {
  protected:
    TestSettings settings{QStringLiteral("QZTests"), QStringLiteral("BodytoneSlopePower")};
    BodytoneSlopeTestBike device;
    CharacteristicWriteProcessor2AD9 processor{1.0, 4, &device, nullptr};

    void SetUp() override {
        settings.activate();
        settings.qsettings.clear();
        settings.qsettings.setValue(QZSettings::weight, 75.0);
        settings.qsettings.setValue(QZSettings::bike_weight, 10.0);
        settings.qsettings.setValue(QZSettings::rolling_resistance, 0.004);
        settings.qsettings.setValue(QZSettings::min_inclination, -20.0);
        settings.qsettings.setValue(QZSettings::zwift_inclination_gain, 1.0);
        settings.qsettings.setValue(QZSettings::zwift_inclination_offset, 0.0);
        settings.qsettings.setValue(QZSettings::bike_power_offset, 0);
        settings.qsettings.setValue(QZSettings::gears_offset, 0.0);
        device.identify(QStringLiteral("AB500SMS-0018"));
        device.setAutoResistance(true);
        QObject::connect(&processor, &CharacteristicWriteProcessor2AD9::ftmsCharacteristicChanged,
                         &device, [this](const QLowEnergyCharacteristic &, const QByteArray &packet) {
                             EXPECT_TRUE(device.receive(packet));
                         });
        QObject::connect(&processor, &CharacteristicWriteProcessor2AD9::changeInclination,
                         &device, &ftmsbike::changeInclination);
    }

    void write(const char *hex, const char *response = "801101") {
        QByteArray reply;
        ASSERT_EQ(CP_OK, processor.writeProcess(0x2AD9, QByteArray::fromHex(hex), reply));
        EXPECT_EQ(QByteArray::fromHex(response), reply);
    }
};
}

TEST_F(BodytoneSlopePowerTest, UsesSharedPhysicsForRecordedAscentAndDescent) {
    // MyWhoosh/QZ trace: +0.98%, +11.48%, back to +0.98%, then -2.01%.
    write("11000062003533");
    ASSERT_TRUE(device.slopeActive());
    const int low = device.pendingPower();
    ASSERT_GT(low, 0);
    write("1100007c043533");
    const int high = device.pendingPower();
    EXPECT_GT(high, low);
    write("11000062003533");
    EXPECT_EQ(low, device.pendingPower());
    write("11000037ff3533");
    EXPECT_LT(device.pendingPower(), low);
    EXPECT_GE(device.pendingPower(), 0);
    EXPECT_FALSE(settings.qsettings.value(QZSettings::zwift_erg).toBool());
}

TEST_F(BodytoneSlopePowerTest, RefreshesWhenSpeedChangesWithoutAnotherGradeFrame) {
    write("110000f4012833");
    const int initial = device.pendingPower();
    device.speedForTest(25.0);
    device.refreshSlope();
    EXPECT_GT(device.pendingPower(), initial);
}

TEST_F(BodytoneSlopePowerTest, NativeErgReplacesSimAndCanReturnToSim) {
    write("110000f4012833");
    ASSERT_TRUE(device.slopeActive());
    write("056400", "800501"); // native 100 W target
    EXPECT_FALSE(device.slopeActive());
    EXPECT_EQ(100, device.pendingPower());
    EXPECT_TRUE(settings.qsettings.value(QZSettings::zwift_erg).toBool());
    write("11000062003533");
    EXPECT_TRUE(device.slopeActive());
    EXPECT_FALSE(settings.qsettings.value(QZSettings::zwift_erg).toBool());
}

TEST_F(BodytoneSlopePowerTest, StopOrResetDiscardsPendingSimTarget) {
    write("110000f4012833");
    ASSERT_TRUE(device.slopeActive());
    ASSERT_TRUE(device.receive(QByteArray::fromHex("0801")));
    EXPECT_FALSE(device.slopeActive());
    EXPECT_EQ(-1, device.pendingPower());
    write("11000062003533");
    ASSERT_TRUE(device.receive(QByteArray::fromHex("01")));
    EXPECT_FALSE(device.slopeActive());
    EXPECT_EQ(-1, device.pendingPower());
}

TEST_F(BodytoneSlopePowerTest, AutoResistanceOffDoesNotActivateSim) {
    device.setAutoResistance(false);
    write("110000f4012833");
    EXPECT_FALSE(device.slopeActive());
    EXPECT_EQ(-1, device.pendingPower());
}

TEST_F(BodytoneSlopePowerTest, OtherFtmsBikesNeverActivateSimToErg) {
    device.identify(QStringLiteral("SUITO"));
    write("110000f4012833");
    EXPECT_FALSE(device.slopeActive());
    EXPECT_EQ(-1, device.pendingPower());
}

TEST_F(BodytoneSlopePowerTest, MalformedSimDoesNotActivateModel) {
    ASSERT_TRUE(device.receive(QByteArray::fromHex("110000")));
    EXPECT_FALSE(device.slopeActive());
    EXPECT_EQ(-1, device.pendingPower());
}

TEST(BodytoneSlopePowerModelTest, OnlyMatchesActive500ModelNames) {
    EXPECT_TRUE(ftmsbike::usesSlopePowerForDevice(QStringLiteral("AB500SMS-0018")));
    EXPECT_TRUE(ftmsbike::usesSlopePowerForDevice(QStringLiteral("ab500sms-1234")));
    EXPECT_TRUE(ftmsbike::usesSlopePowerForDevice(QStringLiteral("AB500SMS")));
    EXPECT_FALSE(ftmsbike::usesSlopePowerForDevice(QStringLiteral("AB500SMS2-0018")));
    EXPECT_FALSE(ftmsbike::usesSlopePowerForDevice(QStringLiteral("Bodytone XS08")));
    EXPECT_FALSE(ftmsbike::usesSlopePowerForDevice(QStringLiteral("MX-RB 123")));
    EXPECT_FALSE(ftmsbike::usesSlopePowerForDevice(QStringLiteral("SUITO")));
}
