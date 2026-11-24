#include <gtest/gtest.h>
#include "devices/ftmsrower/ftmsrower.h"
#include "virtualdevices/virtualrower.h"

namespace {
class ReplayRower : public ftmsrower {
public:
    ReplayRower() : ftmsrower(true, true) {}
    void concept2Stroke(const QByteArray &packet) {
        parseConcept2Data(QBluetoothUuid(QStringLiteral("ce060036-43e5-11e4-916c-0800200c9a66")), packet);
    }
    void concept2Idle() {
        parseConcept2Data(QBluetoothUuid(QStringLiteral("ce060031-43e5-11e4-916c-0800200c9a66")), QByteArray(10, '\0'));
    }
    void ftmsStroke(uint16_t count) { updatePM5StrokeCount(count, PM5StrokeSource::Ftms); }
    void advanceDistance(double km) { Distance += km; }
    void setHardwareDistance(double km) { Distance = km; }
};
uint32_t decodeDistance(const QByteArray &field) {
    return static_cast<uint8_t>(field.at(0)) |
           (static_cast<uint32_t>(static_cast<uint8_t>(field.at(1))) << 8) |
           (static_cast<uint32_t>(static_cast<uint8_t>(field.at(2))) << 16);
}
}

TEST(RowerWorkoutCounters, ReplaysIssue5933AcrossTwoClasses) {
    ReplayRower device;
    virtualrower::FtmsDistance distance;
    // CE060036 notifications from the attached #5933 log, 23 Nov 2026.
    device.concept2Stroke(QByteArray::fromHex("7d b3 02 48 00 24 02 b4 02 00 00 00 46 16 00")); // 20:00:06, 692
    device.concept2Stroke(QByteArray::fromHex("8c b4 02 49 00 26 02 b5 02 00 00 00 47 16 00")); // 20:00:08, 693
    EXPECT_DOUBLE_EQ(693, device.currentStrokesCount().value());
    device.advanceDistance(5.62404); // 20:00:09
    const auto previous = decodeDistance(distance.field(device));
    EXPECT_EQ(5624u, previous);

    device.clearStats(); // start of next QZ workout
    EXPECT_DOUBLE_EQ(0, device.currentStrokesCount().value());
    EXPECT_DOUBLE_EQ(0, device.odometer());
    EXPECT_EQ(previous, decodeDistance(distance.field(device)));

    device.concept2Stroke(QByteArray::fromHex("5a b7 02 17 00 7b 01 b6 02 00 00 00 30 16 00")); // 20:02:01, 694
    EXPECT_DOUBLE_EQ(1, device.currentStrokesCount().value());
    device.advanceDistance(0.00500791); // 20:02:01
    EXPECT_EQ(5629u, decodeDistance(distance.field(device)));

    const auto packet = QByteArray::fromHex("5c be 02 22 00 9f 01 bb 02 00 00 00 39 16 00"); // 20:02:18, 699
    device.concept2Stroke(packet);
    device.concept2Stroke(packet); // duplicate notification
    EXPECT_DOUBLE_EQ(6, device.currentStrokesCount().value());
    device.concept2Stroke(QByteArray::fromHex("50 2d 03 3e 00 00 02 23 03 00 00 00 2a 2a 00")); // 20:07:02, 803
    EXPECT_DOUBLE_EQ(110, device.currentStrokesCount().value());
    device.advanceDistance(0.900035 - 0.00500791);
    EXPECT_NEAR(0.900035, device.odometer(), 1e-8);
    EXPECT_EQ(6524u, decodeDistance(distance.field(device)));
}

TEST(RowerWorkoutCounters, HandlesHardwareResetAndFullDistanceField) {
    ReplayRower device;
    virtualrower::FtmsDistance distance;
    device.concept2Stroke(QByteArray::fromHex("00 00 00 00 00 00 00 64 00"));
    device.clearStats();
    device.concept2Stroke(QByteArray::fromHex("00 00 00 00 00 00 00 00 00"));
    device.concept2Stroke(QByteArray::fromHex("00 00 00 00 00 00 00 01 00"));
    EXPECT_DOUBLE_EQ(1, device.currentStrokesCount().value());
    device.advanceDistance(70.001);
    EXPECT_EQ(70001u, decodeDistance(distance.field(device)));
}

TEST(RowerWorkoutCounters, CountsConcept2StrokesBeforeRowStateAndIgnoresSecondSource) {
    ReplayRower device;
    device.concept2Stroke(QByteArray::fromHex("00 00 00 00 00 00 00 64 00"));
    device.clearStats();
    device.concept2Idle();
    device.concept2Stroke(QByteArray::fromHex("00 00 00 00 00 00 00 65 00"));
    device.ftmsStroke(101);
    EXPECT_DOUBLE_EQ(1, device.currentStrokesCount().value());
}

TEST(RowerWorkoutCounters, NormalizesFtmsWhenItIsTheFirstPm5StrokeSource) {
    ReplayRower device;
    device.ftmsStroke(100);
    device.clearStats();
    device.concept2Stroke(QByteArray::fromHex("00 00 00 00 00 00 00 66 00"));
    device.ftmsStroke(101);
    EXPECT_DOUBLE_EQ(1, device.currentStrokesCount().value());
}

TEST(RowerWorkoutCounters, KeepsVirtualDistanceAcrossHardwareResetAndRebasesOnConnect) {
    ReplayRower device;
    virtualrower::FtmsDistance distance;
    device.setHardwareDistance(5.625);
    EXPECT_EQ(5625u, decodeDistance(distance.field(device)));
    device.clearStats();
    EXPECT_EQ(5625u, decodeDistance(distance.field(device)));
    device.setHardwareDistance(0);
    EXPECT_EQ(5625u, decodeDistance(distance.field(device)));
    device.setHardwareDistance(0.125);
    EXPECT_EQ(5750u, decodeDistance(distance.field(device)));

    device.clearStats();
    distance = virtualrower::FtmsDistance{};
    EXPECT_EQ(0u, decodeDistance(distance.field(device)));
    device.setHardwareDistance(0.375);
    EXPECT_EQ(250u, decodeDistance(distance.field(device)));
}
