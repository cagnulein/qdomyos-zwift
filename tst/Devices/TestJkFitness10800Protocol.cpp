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
    EXPECT_EQ(heartbeatFrame(), QByteArray::fromHex("55170101"));
    EXPECT_EQ(telemetryQueryFrame(), QByteArray::fromHex("550701ff"));
    EXPECT_EQ(profileFrame(42, true, 178.5, 81.2), QByteArray::fromHex("5501062a005114b232"));
    EXPECT_EQ(speedFrame(8.0, 1), QByteArray::fromHex("550f020461"));
    EXPECT_EQ(profileFrame(42, false, 178.0, 81.0, 1), QByteArray::fromHex("5501062a01b2394608"));
}

TEST(JkFitness10800Protocol, BuildsNativeConnectionQuerySequence) {
    const QVector<QByteArray> frames = initializationFrames();
    const QVector<QByteArray> expected = {
        QByteArray::fromHex("550c01ff"), QByteArray::fromHex("55bb01ff"), QByteArray::fromHex("552401ff"),
        QByteArray::fromHex("552501ff"), QByteArray::fromHex("552601ff"), QByteArray::fromHex("552701ff"),
        QByteArray::fromHex("550201ff"), QByteArray::fromHex("550301ff"), QByteArray::fromHex("550401ff"),
        QByteArray::fromHex("550601ff"), QByteArray::fromHex("551f01ff"), QByteArray::fromHex("55a001ff"),
        QByteArray::fromHex("55b001ff"), QByteArray::fromHex("55b201ff"), QByteArray::fromHex("55b301ff"),
        QByteArray::fromHex("55b401ff"), QByteArray::fromHex("55b501ff"), QByteArray::fromHex("55b601ff"),
        QByteArray::fromHex("55b701ff"), QByteArray::fromHex("55b801ff"), QByteArray::fromHex("55b901ff"),
        QByteArray::fromHex("55ba01ff"), QByteArray::fromHex("550b01ff"), QByteArray::fromHex("551801ff"),
        QByteArray::fromHex("551901ff"), QByteArray::fromHex("551a01ff"), QByteArray::fromHex("551b01ff")};
    EXPECT_EQ(frames, expected);
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
    EXPECT_TRUE(isConsoleState(QByteArray::fromHex("55090102"), running));
    EXPECT_FALSE(running);
    bool paused = false;
    EXPECT_TRUE(isConsoleState(QByteArray::fromHex("55090100"), running, paused));
    EXPECT_FALSE(running);
    EXPECT_TRUE(paused);
    EXPECT_TRUE(isConsoleState(QByteArray::fromHex("55090102"), running, paused));
    EXPECT_FALSE(running);
    EXPECT_FALSE(paused);
}

TEST(JkFitness10800Protocol, ValidatesNativeTelemetryOpcodeLengthAndType) {
    QByteArray shortTelemetry = QByteArray::fromHex("550d0a00000000000008320600");
    double speed = 0.0;
    double incline = 0.0;
    ASSERT_TRUE(decodeTelemetry(shortTelemetry, 2, 0, speed, incline));
    EXPECT_DOUBLE_EQ(speed, 8.50);
    EXPECT_DOUBLE_EQ(incline, 6.0);
    EXPECT_TRUE(decodeTelemetry(shortTelemetry, 2, 1, speed, incline));
    EXPECT_NEAR(speed, 8.50 / 0.62137, 0.00001);
    EXPECT_FALSE(decodeTelemetry(shortTelemetry, 1, 0, speed, incline));

    QByteArray extendedTelemetry = QByteArray::fromHex("550d0d00000000000008320600000000");
    EXPECT_EQ(extendedTelemetry.size(), 16);
    EXPECT_TRUE(decodeTelemetry(extendedTelemetry, 0, 0, speed, incline));
    EXPECT_FALSE(decodeTelemetry(QByteArray::fromHex("550c0a00000000000008320600"), 2, 0, speed, incline));
    EXPECT_FALSE(decodeTelemetry(QByteArray::fromHex("550d0900000000000008320600"), 2, 0, speed, incline));

    shortTelemetry[9] = static_cast<char>(0x01);
    shortTelemetry[10] = static_cast<char>(0x00);
    ASSERT_TRUE(decodeTelemetry(shortTelemetry, 2, 0, speed, incline));
    EXPECT_DOUBLE_EQ(speed, 1.0);
}

TEST(JkFitness10800Protocol, DecodesNativeProtocolDiscoveryAndUnits) {
    int value = -1;
    EXPECT_TRUE(decodeDeviceType(QByteArray::fromHex("55bb0102"), value));
    EXPECT_EQ(value, 2);
    EXPECT_FALSE(decodeDeviceType(QByteArray::fromHex("55bb0103"), value));
    EXPECT_TRUE(decodeUnits(QByteArray::fromHex("550c0101"), value));
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(isRetryDeviceTypeQuery(QByteArray::fromHex("55bb00")));
}

TEST(JkFitness10800Protocol, DecodesNativeSecondaryTelemetryByDeviceType) {
    double incline = 0.0;
    double speed = 0.0;
    const QByteArray treadmillStatus = QByteArray::fromHex("551d0a00000000000008320600");
    EXPECT_TRUE(decodeTelemetry(treadmillStatus, 2, 0, speed, incline));
    EXPECT_DOUBLE_EQ(speed, 8.50);
    EXPECT_DOUBLE_EQ(incline, 6.0);
    const QByteArray extendedStatus = QByteArray::fromHex("551d07080604ff000000");
    EXPECT_TRUE(decodeTelemetry(extendedStatus, 1, 0, speed, incline));
    EXPECT_DOUBLE_EQ(speed, 8.06);
    EXPECT_DOUBLE_EQ(incline, 4.0);
    EXPECT_FALSE(decodeTelemetry(extendedStatus, 2, 0, speed, incline));
}

TEST(JkFitness10800Protocol, DoesNotInterpretKeepAliveAsTelemetry) {
    double speed = 0.0;
    double incline = 0.0;
    EXPECT_FALSE(decodeTelemetry(keepAliveFrame(), speed, incline));
}
