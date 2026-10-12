#include <gtest/gtest.h>

#include "devices/fitpro/fitprocodec.h"

namespace {

QByteArray packet(const QList<QByteArray> &packets, int index) {
    EXPECT_LT(index, packets.size());
    return index < packets.size() ? packets.at(index) : QByteArray();
}

} // namespace

TEST(ProformFitProCodecTest, GeneratesTreadmillSpeedFrameFromValue) {
    const QList<QByteArray> packets = fitpro::Codec::speedCommand(7.0);

    ASSERT_EQ(packets.size(), 2);
    EXPECT_EQ(packet(packets, 0), QByteArray::fromHex("fe020d02"));
    EXPECT_EQ(packet(packets, 1), QByteArray::fromHex("ff0d020402090409020101bc0200cf0000000000"));
}

TEST(ProformFitProCodecTest, GeneratesTreadmillGradeFrameFromValue) {
    const QList<QByteArray> packets = fitpro::Codec::gradeCommand(7.0);

    ASSERT_EQ(packets.size(), 2);
    EXPECT_EQ(packet(packets, 0), QByteArray::fromHex("fe020d02"));
    EXPECT_EQ(packet(packets, 1), QByteArray::fromHex("ff0d020402090409020102bc0200d00000000000"));
}

TEST(ProformFitProCodecTest, GeneratesBikeResistanceFrameWithoutProfileArray) {
    const QList<QByteArray> packets = fitpro::Codec::rawResistanceCommand(0x0384, fitpro::Codec::FitnessBike);

    ASSERT_EQ(packets.size(), 2);
    EXPECT_EQ(packet(packets, 0), QByteArray::fromHex("fe020d02"));
    EXPECT_EQ(packet(packets, 1), QByteArray::fromHex("ff0d0204020907090201048403009e0000000000"));
}

TEST(ProformFitProCodecTest, GeneratesSpinBikeGradeFrameWithDeviceEight) {
    const QList<QByteArray> packets = fitpro::Codec::gradeCommand(2.0, fitpro::Codec::SpinBike);

    ASSERT_EQ(packets.size(), 2);
    EXPECT_EQ(packet(packets, 1), QByteArray::fromHex("ff0d020402090809020102c80000de0000000000"));
}

TEST(ProformFitProCodecTest, GeneratesInclineTrainerSpeedWithDeviceFive) {
    const QList<QByteArray> packets = fitpro::Codec::speedCommand(7.0, fitpro::Codec::InclineTrainer);

    ASSERT_EQ(packets.size(), 2);
    EXPECT_EQ(packet(packets, 1), QByteArray::fromHex("ff0d020402090509020101bc0200d00000000000"));
}

TEST(ProformFitProCodecTest, GeneratesDiscoveryCommandsUsedByTheApk) {
    const QList<QByteArray> info = fitpro::Codec::discovery(fitpro::Codec::GetInfo);
    const QList<QByteArray> supportedDevices = fitpro::Codec::discovery(fitpro::Codec::GetSupportedDevices);
    const QList<QByteArray> supportedCommands = fitpro::Codec::discovery(fitpro::Codec::GetSupportedCommands);

    ASSERT_EQ(info.size(), 2);
    EXPECT_EQ(packet(info, 0), QByteArray::fromHex("fe020802"));
    EXPECT_EQ(packet(info, 1), QByteArray::fromHex("ff08020402040204818700000000000000000000"));
    EXPECT_EQ(packet(supportedDevices, 1), QByteArray::fromHex("ff08020402040204808600000000000000000000"));
    EXPECT_EQ(packet(supportedCommands, 1), QByteArray::fromHex("ff08020402040204888e00000000000000000000"));
}

TEST(ProformFitProCodecTest, PacksFieldsInSortedOrderAndUsesSectionMasks) {
    const QByteArray frame = fitpro::Codec::writeReadFrame(
        fitpro::Codec::Treadmill,
        {{fitpro::Codec::FieldGrade, QByteArray::fromHex("5802")},
         {fitpro::Codec::FieldSpeed, QByteArray::fromHex("a000")}},
        {fitpro::Codec::FieldWorkoutMode});

    EXPECT_EQ(frame, QByteArray::fromHex("040d020103a000580202001023"));
}

TEST(ProformFitProCodecTest, GeneratesLongWorkoutModeReadWithCorrectFragmentation) {
    const QList<QByteArray> packets = fitpro::Codec::workoutModeRead(fitpro::Codec::FitnessBike);

    ASSERT_EQ(packets.size(), 2);
    EXPECT_EQ(packet(packets, 0), QByteArray::fromHex("fe020c02"));
    EXPECT_EQ(packet(packets, 1), QByteArray::fromHex("ff0c020402080708020002001023000000000000"));
}

TEST(ProformFitProCodecTest, GeneratesRequiredStartFieldThroughGenericBitfieldPath) {
    const QList<QByteArray> packets = fitpro::Codec::requiredStartCommand(true);

    ASSERT_EQ(packets.size(), 3);
    EXPECT_EQ(packet(packets, 0), QByteArray::fromHex("fe021903"));
    EXPECT_EQ(packet(packets, 1), QByteArray::fromHex("0012020402150415020e00000000000000000000"));
    EXPECT_EQ(packet(packets, 2), QByteArray::fromHex("ff070000001001003a0000000000000000000000"));
}

TEST(ProformFitProCodecTest, ReassemblesAndDecodesGenericTelemetryFrame) {
    const QList<QByteArray> packets = fitpro::Codec::speedCommand(1.6);
    const QByteArray wrapped = fitpro::Codec::reassemble(packets);
    ASSERT_FALSE(wrapped.isEmpty());

    fitpro::Frame frame;
    ASSERT_TRUE(fitpro::Codec::decode(fitpro::Codec::unwrap(wrapped), frame));
    ASSERT_EQ(frame.device, fitpro::Codec::Treadmill);
    ASSERT_EQ(frame.command, fitpro::Codec::WriteReadData);
    ASSERT_EQ(frame.fields.size(), 1);
    EXPECT_EQ(frame.fields.at(0).id, fitpro::Codec::FieldSpeed);
    EXPECT_EQ(frame.fields.at(0).value, QByteArray::fromHex("a000"));
}

TEST(ProformFitProCodecTest, RejectsCorruptFragmentOrChecksum) {
    QList<QByteArray> packets = fitpro::Codec::gradeCommand(3.0);
    packets[1][0] = static_cast<char>(0x01);
    EXPECT_TRUE(fitpro::Codec::reassemble(packets).isEmpty());

    packets = fitpro::Codec::gradeCommand(3.0);
    packets[1][4] = static_cast<char>(0x00);
    const QByteArray wrapped = fitpro::Codec::reassemble(packets);
    fitpro::Frame frame;
    EXPECT_FALSE(fitpro::Codec::decode(fitpro::Codec::unwrap(wrapped), frame));
}
