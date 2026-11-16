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

TEST(KettlerClassicProtocol, CoachSDecodesCandidatePowerFromRealE0Telemetry) {
    // Real 24-byte E0 payloads from Jacek's successful 2026-11-16
    // Coach S test. Power is provisional until checked against the console.
    const QByteArray idle = QByteArray::fromHex(
        "000400000000000000000000000000000000000000000001");
    const QByteArray w80 = QByteArray::fromHex(
        "00040000000000000000000009ac023d000000500ab30001");
    const QByteArray w170 = QByteArray::fromHex(
        "0004000400000041006f045829e30453002400aa16520001");
    const QByteArray w210 = QByteArray::fromHex(
        "000400010000003100890457228a0506000e00d21ab50001");
    ASSERT_EQ(idle.size(), 24);
    ASSERT_EQ(w80.size(), 24);
    ASSERT_EQ(w170.size(), 24);
    ASSERT_EQ(w210.size(), 24);
    EXPECT_EQ(kettlerclassicprotocol::coachSCandidateWatts(idle), 0);
    EXPECT_EQ(kettlerclassicprotocol::coachSCandidateWatts(w80), 80);
    EXPECT_EQ(kettlerclassicprotocol::coachSCandidateWatts(w170), 170);
    EXPECT_EQ(kettlerclassicprotocol::coachSCandidateWatts(w210), 210);

    QByteArray wrongType = w170;
    wrongType[1] = char(0x05);
    EXPECT_EQ(kettlerclassicprotocol::coachSCandidateWatts(wrongType), 0);
    EXPECT_EQ(kettlerclassicprotocol::coachSCandidateWatts(w170.left(20)), 0);
}

TEST(KettlerClassicProtocol, CoachSKinomapTargetLevelMapping) {
    // Coach S has ten levels; 1..8 are user-reported Kinomap target W.
    // L9/L10 must be confirmed by a new controlled hardware test.
    const int observedWatts[] = {50, 60, 80, 100, 125, 145, 165, 185};
    for (int level = 1; level <= 8; ++level) {
        EXPECT_EQ(kettlerclassicprotocol::coachSTargetWattsForLevel(level),
                  observedWatts[level - 1]);
    }
    EXPECT_EQ(kettlerclassicprotocol::coachSTargetWattsForLevel(9), 205);
    EXPECT_EQ(kettlerclassicprotocol::coachSTargetWattsForLevel(10), 225);
    EXPECT_EQ(kettlerclassicprotocol::coachSTargetWattsForLevel(-1), 50);
    EXPECT_EQ(kettlerclassicprotocol::coachSTargetWattsForLevel(99), 225);
}

TEST(KettlerClassicProtocol, CoachSEncodesTargetPowerWithEscapingAndCrc) {
    // 0x00C2/0x02, U16 big-endian target W. For 50 W the 0x02
    // operation AND 0x02 payload-length byte must be escaped.
    const QByteArray watts50 = QByteArray::fromHex("0032");
    const QByteArray frame50 =
        kettlerclassicprotocol::encodeFrame(0x00c2, 0x02, watts50);
    EXPECT_EQ(frame50, QByteArray::fromHex("0200c21022001022003203be84"));

    // The same operation at level L8 (185 W), using the verified shared
    // frame/CRC encoder rather than reusing any quoted unverified CRC.
    const quint16 w185 = kettlerclassicprotocol::coachSTargetWattsForLevel(8);
    QByteArray watts185;
    watts185.append(static_cast<char>(w185 >> 8));
    watts185.append(static_cast<char>(w185 & 0xff));
    const QByteArray frame185 =
        kettlerclassicprotocol::encodeFrame(0x00c2, 0x02, watts185);
    EXPECT_EQ(frame185, QByteArray::fromHex("0200c2102200102200b903845f"));

    QByteArray decoded = frame50 + frame185 +
        kettlerclassicprotocol::encodeFrame(0x00c2, 0x03, QByteArray());
    const auto frames = kettlerclassicprotocol::consumeFrames(decoded);
    ASSERT_EQ(frames.size(), 3);
    EXPECT_EQ(frames.at(0).function, 0x00c2);
    EXPECT_EQ(frames.at(0).subFunction, 0x02);
    EXPECT_EQ(frames.at(0).payload, watts50);
    EXPECT_EQ(frames.at(1).payload, watts185);
    EXPECT_EQ(frames.at(2).function, 0x00c2);
    EXPECT_EQ(frames.at(2).subFunction, 0x03);
    EXPECT_TRUE(frames.at(2).payload.isEmpty());
    EXPECT_TRUE(decoded.isEmpty());
}
