#include <gtest/gtest.h>

#include <QByteArray>

#include "devices/kettlerclassicbike/kettlerclassicprotocol.h"

using kettlerclassicprotocol::Frame;

TEST(KettlerClassicProtocol, EncodesMyHomeFitFrameWithEscapingAndCrc) {
    const QByteArray payload = QByteArray::fromHex("02031034");
    const QByteArray encoded = kettlerclassicprotocol::encodeFrame(0x0102, 0x01, payload);

    EXPECT_EQ(encoded, QByteArray::fromHex("020110220100041022102310303403c073"));
}

TEST(KettlerClassicProtocol, EncodesMyHomeFitHandshake) {
    const QByteArray handshakePayload = QByteArray::fromHex("2d24f22496a4ff9829f921be9daa9e4d0117");
    const QByteArray encoded = kettlerclassicprotocol::encodeFrame(0x0001, 0x01, handshakePayload);

    EXPECT_EQ(encoded, QByteArray::fromHex("0200010100122d24f22496a4ff9829f921be9daa9e4d0117038677"));
}

TEST(KettlerClassicProtocol, DecodesFragmentedAndConcatenatedFrames) {
    const QByteArray handshake = kettlerclassicprotocol::encodeFrame(
        0x0001, 0x01, QByteArray::fromHex("2d24f22496a4ff9829f921be9daa9e4d0117"));
    const QByteArray telemetry = kettlerclassicprotocol::encodeFrame(0x0009, 0x03, QByteArray::fromHex("0050"));
    QByteArray stream = QByteArray::fromHex("ff 00") + handshake.left(7);

    EXPECT_TRUE(kettlerclassicprotocol::consumeFrames(stream).isEmpty());
    stream.append(handshake.mid(7));
    stream.append(telemetry);
    const QVector<Frame> frames = kettlerclassicprotocol::consumeFrames(stream);

    ASSERT_EQ(frames.size(), 2);
    EXPECT_EQ(frames.at(0).function, 0x0001);
    EXPECT_EQ(frames.at(0).subFunction, 0x01);
    EXPECT_EQ(frames.at(0).payload, QByteArray::fromHex("2d24f22496a4ff9829f921be9daa9e4d0117"));
    EXPECT_EQ(frames.at(1).function, 0x0009);
    EXPECT_EQ(frames.at(1).subFunction, 0x03);
    EXPECT_EQ(frames.at(1).payload, QByteArray::fromHex("0050"));
    EXPECT_TRUE(stream.isEmpty());
}

TEST(KettlerClassicProtocol, RejectsBadCrcAndKeepsFollowingFrame) {
    QByteArray bad = kettlerclassicprotocol::encodeFrame(0x0009, 0x03, QByteArray::fromHex("0050"));
    bad[bad.size() - 1] = static_cast<char>(static_cast<unsigned char>(bad.at(bad.size() - 1)) ^ 0x01);
    const QByteArray good = kettlerclassicprotocol::encodeFrame(0x000b, 0x03, QByteArray::fromHex("012c"));

    QByteArray stream = bad + good;
    const QVector<Frame> frames = kettlerclassicprotocol::consumeFrames(stream);

    ASSERT_EQ(frames.size(), 1);
    EXPECT_EQ(frames.at(0).function, 0x000b);
    EXPECT_EQ(frames.at(0).payload, QByteArray::fromHex("012c"));
    EXPECT_TRUE(stream.isEmpty());
}

TEST(KettlerClassicProtocol, PreservesIncompleteEscapedByte) {
    const QByteArray frame = kettlerclassicprotocol::encodeFrame(0x001b, 0x02, QByteArray(1, '\x02'));
    QByteArray stream = frame.left(frame.indexOf(char(0x10)) + 1);

    EXPECT_TRUE(kettlerclassicprotocol::consumeFrames(stream).isEmpty());
    stream.append(frame.mid(stream.size()));
    const QVector<Frame> frames = kettlerclassicprotocol::consumeFrames(stream);

    ASSERT_EQ(frames.size(), 1);
    EXPECT_EQ(frames.at(0).function, 0x001b);
    EXPECT_EQ(frames.at(0).payload, QByteArray(1, '\x02'));
}

TEST(KettlerClassicProtocol, CoachSCapturedHandshakeMatchesWireBytes) {
    // 2026-11-15 Sony Xperia L1 HCI snoop, successful Kinomap/Coach S session.
    const QByteArray challenge = QByteArray::fromHex("2d61bc3de75ad3dfe2973ef18a637da428d6");
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(0x0001, 0x01, challenge),
              QByteArray::fromHex("0200010100122d61bc3de75ad3dfe2973ef18a637da428d603ba96"));

    QByteArray response = QByteArray::fromHex(
        "020001102300127849dfe4e0a14e4f14451817bf974991a44c03ed52");
    const auto frames = kettlerclassicprotocol::consumeFrames(response);
    ASSERT_EQ(frames.size(), 1);
    EXPECT_EQ(frames.at(0).function, 0x0001);
    EXPECT_EQ(frames.at(0).subFunction, 0x03);
    EXPECT_EQ(frames.at(0).payload, QByteArray::fromHex("7849dfe4e0a14e4f14451817bf974991a44c"));
    EXPECT_TRUE(response.isEmpty());
}

TEST(KettlerClassicProtocol, CoachSCapturedRowingTelemetryDecodesAcrossFragments) {
    // Captured E0 frame contains escaped 0x02 inside the rowing-data block.
    const QByteArray captured = QByteArray::fromHex(
        "0200e0102300180004000d0026008800b81022ec2f371022ca004a013423d8000403a889");
    QByteArray stream = captured.left(13);
    EXPECT_TRUE(kettlerclassicprotocol::consumeFrames(stream).isEmpty());
    stream.append(captured.mid(13));
    stream.append(QByteArray::fromHex("0200d3010000038769"));
    const auto frames = kettlerclassicprotocol::consumeFrames(stream);
    ASSERT_EQ(frames.size(), 2);
    EXPECT_EQ(frames.at(0).function, 0x00e0);
    EXPECT_EQ(frames.at(0).subFunction, 0x03);
    EXPECT_EQ(frames.at(0).payload.size(), 24);
    EXPECT_EQ(frames.at(0).payload,
              QByteArray::fromHex("0004000d0026008800b802ec2f3702ca004a013423d80004"));
    EXPECT_EQ(frames.at(1).function, 0x00d3);
    EXPECT_EQ(frames.at(1).subFunction, 0x01);
    EXPECT_TRUE(stream.isEmpty());
}

TEST(KettlerClassicProtocol, CoachSThirdKinomapChallengeCountsEscapedWirePayloadBytes) {
    // Third independent successful Kinomap HCI capture, 2026-11-15.
    // The 18 decoded challenge bytes contain 0x10, 0x02 and 0x10, each escaped
    // on the wire. Kinomap advertises 0x0015 (21) and CRC 0x602B, not 0x0012.
    const QByteArray challenge = QByteArray::fromHex("927c6464e25210a2a48e02fa7e9006107599");
    const QByteArray captured = QByteArray::fromHex(
        "020001010015927c6464e2521030a2a48e1022fa7e90061030759903602b");
    EXPECT_EQ(challenge.size(), 18);
    EXPECT_EQ(kettlerclassicprotocol::encodeFrameWithEscapedPayloadLength(0x0001, 0x01, challenge),
              captured);

    // Coach S response still advertises decoded payload length (18); the
    // escaped 0x03 in the CRC is handled by the standard response decoder.
    QByteArray response = QByteArray::fromHex(
        "02000110230012fbbe2e001bcda9e3a53b52c1847f276449bd0310236a");
    const auto frames = kettlerclassicprotocol::consumeFrames(response);
    ASSERT_EQ(frames.size(), 1);
    EXPECT_EQ(frames.at(0).function, 0x0001);
    EXPECT_EQ(frames.at(0).subFunction, 0x03);
    EXPECT_EQ(frames.at(0).payload,
              QByteArray::fromHex("fbbe2e001bcda9e3a53b52c1847f276449bd"));
    EXPECT_TRUE(response.isEmpty());
}

TEST(KettlerClassicProtocol, CoachSAuthenticationMatchesSixKinomapVectors) {
    // Three complete and independent successful Kinomap sessions, each with
    // a client challenge and a Coach S reply. Both are salt + MD5(salt || key).
    struct Vector { const char *request; const char *reply; };
    const Vector vectors[] = {
        {"2d61bc3de75ad3dfe2973ef18a637da428d6", "7849dfe4e0a14e4f14451817bf974991a44c"},
        {"5fb8892e145cb599a16e1832573e3384b6bb", "45034120c3281fcdd2c01ffae49e62b72415"},
        {"927c6464e25210a2a48e02fa7e9006107599", "fbbe2e001bcda9e3a53b52c1847f276449bd"}
    };
    for (const auto &v : vectors) {
        const QByteArray request = QByteArray::fromHex(v.request);
        const QByteArray reply = QByteArray::fromHex(v.reply);
        ASSERT_EQ(request.size(), 18);
        ASSERT_EQ(reply.size(), 18);
        EXPECT_EQ(kettlerclassicprotocol::coachSHandshakeForSalt(request.left(2)), request);
        EXPECT_EQ(kettlerclassicprotocol::coachSHandshakeForSalt(reply.left(2)), reply);
        EXPECT_TRUE(kettlerclassicprotocol::isValidCoachSHandshake(request));
        EXPECT_TRUE(kettlerclassicprotocol::isValidCoachSHandshake(reply));
        QByteArray tampered = reply;
        tampered[17] = static_cast<char>(tampered.at(17) ^ 0x01);
        EXPECT_FALSE(kettlerclassicprotocol::isValidCoachSHandshake(tampered));
    }
    EXPECT_TRUE(kettlerclassicprotocol::coachSHandshakeForSalt(QByteArray("a")).isEmpty());
    EXPECT_FALSE(kettlerclassicprotocol::isValidCoachSHandshake(QByteArray(18, char(0))));
}

TEST(KettlerClassicProtocol, CoachSOptionalPowerAndBrakeIrmpCommands) {
    // Verified S-FIT logical command layout, encoded using the existing
    // Kettler Classic STX/ETX/CRC protocol. Coach S device support is probed.
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(0x000b, 0x01, QByteArray()),
              QByteArray::fromHex("02000b010000039ac9"));
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(0x001b, 0x01, QByteArray()),
              QByteArray::fromHex("02001b010000035968"));
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(0x001c, 0x01, QByteArray()),
              QByteArray::fromHex("02001c010000030e49"));
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(0x001d, 0x01, QByteArray()),
              QByteArray::fromHex("02001d0100000312f2"));

    // Subfunction 0x02 is escaped because it matches STX; level 0x10
    // also needs escaping. Do not use the special Coach S handshake encoder.
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(0x001b, 0x02,
                                                   QByteArray(1, char(0x10))),
              QByteArray::fromHex("02001b10220001103003c383"));
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(0x001b, 0x02,
                                                   QByteArray(1, char(5))),
              QByteArray::fromHex("02001b10220001050384af"));

    QByteArray input = QByteArray::fromHex(
        "02000b1023001022007b03480502001b102300010f03374e");
    const auto frames = kettlerclassicprotocol::consumeFrames(input);
    ASSERT_EQ(frames.size(), 2);
    EXPECT_EQ(frames.at(0).function, 0x000b);
    EXPECT_EQ(frames.at(0).subFunction, 0x03);
    EXPECT_EQ(frames.at(0).payload, QByteArray::fromHex("007b")); // 123 watts
    EXPECT_EQ(frames.at(1).function, 0x001b);
    EXPECT_EQ(frames.at(1).subFunction, 0x03);
    EXPECT_EQ(frames.at(1).payload, QByteArray::fromHex("0f")); // level 15
    EXPECT_TRUE(input.isEmpty());
}
