#include <gtest/gtest.h>

#include "devices/jkfitness10800treadmill/jkfitness10800protocol.h"

using namespace jkfitness10800protocol;

TEST(JkFitness10800Protocol, BuildsSourceDerivedControlFrames) {
    EXPECT_EQ(startFrame(), QByteArray::fromHex("550a0101"));
    EXPECT_EQ(stopFrame(), QByteArray::fromHex("550a0102"));
    EXPECT_EQ(pauseFrame(), QByteArray::fromHex("550a0100"));
    EXPECT_EQ(speedFrame(8.32), QByteArray::fromHex("550f020820"));
    EXPECT_EQ(inclineFrame(6.0), QByteArray::fromHex("55110106"));
    EXPECT_EQ(keepAliveFrame().size(), 13);
}

TEST(JkFitness10800Protocol, ClampsControlValues) {
    EXPECT_EQ(speedFrame(-1.0), QByteArray::fromHex("550f020000"));
    EXPECT_EQ(speedFrame(30.0), QByteArray::fromHex("550f021600"));
    EXPECT_EQ(inclineFrame(-2.0), QByteArray::fromHex("55110100"));
    EXPECT_EQ(inclineFrame(30.0), QByteArray::fromHex("5511010f"));
}

TEST(JkFitness10800Protocol, ReassemblesFragmentedAndConcatenatedFrames) {
    QByteArray buffer = QByteArray::fromHex("00ff550f0208");
    QVector<Frame> frames = consumeFrames(buffer);
    ASSERT_TRUE(frames.isEmpty());
    EXPECT_EQ(buffer, QByteArray::fromHex("550f0208"));

    buffer.append(QByteArray::fromHex("32550a0102"));
    frames = consumeFrames(buffer);
    ASSERT_EQ(frames.size(), 2);
    EXPECT_EQ(frames.at(0).raw, QByteArray::fromHex("550f020832"));
    EXPECT_EQ(frames.at(1).raw, QByteArray::fromHex("550a0102"));
    EXPECT_TRUE(buffer.isEmpty());
}

TEST(JkFitness10800Protocol, RejectsMalformedAndIncompleteFrames) {
    QByteArray buffer = QByteArray::fromHex("550f7f550a0101");
    QVector<Frame> frames = consumeFrames(buffer);
    ASSERT_EQ(frames.size(), 1);
    EXPECT_EQ(frames.at(0).raw, QByteArray::fromHex("550a0101"));
    EXPECT_TRUE(buffer.isEmpty());

    buffer = QByteArray::fromHex("550d0a0102");
    frames = consumeFrames(buffer);
    EXPECT_TRUE(frames.isEmpty());
    EXPECT_EQ(buffer, QByteArray::fromHex("550d0a0102"));
}

TEST(JkFitness10800Protocol, DecodesTelemetryAndConsoleState) {
    QByteArray telemetry;
    telemetry.append(static_cast<char>(0x55));
    telemetry.append(static_cast<char>(0x0d));
    telemetry.append(static_cast<char>(0x0a));
    telemetry.append(QByteArray(6, static_cast<char>(0x00)));
    telemetry.append(static_cast<char>(0x08));
    telemetry.append(static_cast<char>(0x32));
    telemetry.append(static_cast<char>(0x06));
    telemetry.append(static_cast<char>(0x00));

    double speed = 0.0;
    double incline = 0.0;
    ASSERT_TRUE(decodeTelemetry(telemetry, speed, incline));
    EXPECT_DOUBLE_EQ(speed, 8.50);
    EXPECT_DOUBLE_EQ(incline, 6.0);

    bool running = false;
    EXPECT_TRUE(isConsoleState(QByteArray::fromHex("55090101"), running));
    EXPECT_TRUE(running);
    EXPECT_TRUE(isConsoleState(QByteArray::fromHex("55090100"), running));
    EXPECT_FALSE(running);
}

TEST(JkFitness10800Protocol, DoesNotInterpretKeepAliveAsTelemetry) {
    double speed = 0.0;
    double incline = 0.0;
    EXPECT_FALSE(decodeTelemetry(keepAliveFrame(), speed, incline));
}
