#include <gtest/gtest.h>

#include <QByteArray>
#include <cmath>

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

TEST(KettlerClassicProtocol, CoachSConsoleWattsUseE0BytesSixSeven) {
    // Physical Coach S, 2026-11-18. Values matched console readings at
    // completed stroke boundaries; bytes 18-19 do not contain those watts.
    const struct { const char *payload; quint16 watts; } captures[] = {
        {"000400280018009100c4046d0000000001ae000000000004", 145}, // old workout
        {"000400290006000c001001e8120200f401b8005013130004", 12},
        {"0004002b008c0011001704b11f8500b601d000500a740004", 17}
    };
    for (const auto &entry : captures) {
        const QByteArray e0 = QByteArray::fromHex(entry.payload);
        ASSERT_EQ(e0.size(), 24);
        EXPECT_EQ(kettlerclassicprotocol::coachSStrokeWatts(e0), entry.watts);
    }
    QByteArray invalid(24, char(0));
    invalid[0] = char(0);
    invalid[1] = char(4);
    invalid[6] = char(0);
    invalid[7] = char(0xc5);
    EXPECT_EQ(kettlerclassicprotocol::coachSStrokeWatts(invalid), 197);
    invalid[1] = char(5);
    EXPECT_EQ(kettlerclassicprotocol::coachSStrokeWatts(invalid), 0);
    EXPECT_EQ(kettlerclassicprotocol::coachSStrokeWatts(invalid.left(20)), 0);
}

TEST(KettlerClassicProtocol, CoachSResistanceWritesMatchKinomapHciCapture) {
    // Kinomap's confirmed physical-load sequence was L4 -> L6 -> L5 ->
    // L3 -> L4. The C0/02 writes have a one-byte resistance-level payload.
    // Escaped payload size (not logical size) is used in the length field.
    struct Sample { quint8 level; const char *wire; };
    const Sample captured[] = {
        {4, "0200c01022000104032dfd"},
        {6, "0200c01022000106030eef"},
        {5, "0200c01022000105033c74"},
        {3, "0200c01022001022102303732a"},
        {4, "0200c01022000104032dfd"}
    };
    for (const auto &sample : captured) {
        const QByteArray wire =
            kettlerclassicprotocol::encodeFrameWithEscapedPayloadLength(
                0x00c0, 0x02, QByteArray(1, static_cast<char>(sample.level)));
        EXPECT_EQ(wire, QByteArray::fromHex(sample.wire))
            << "level=" << static_cast<int>(sample.level);
    }

    // L2's payload is also a control byte; verify escaped-length framing.
    EXPECT_EQ(kettlerclassicprotocol::encodeFrameWithEscapedPayloadLength(
                  0x00c0, 0x02, QByteArray(1, char(2))),
              QByteArray::fromHex("0200c0102200102210220362a3"));
    // Do not change the shared encoder used for normal Classic requests.
    EXPECT_EQ(kettlerclassicprotocol::encodeFrame(
                  0x00c0, 0x02, QByteArray(1, char(3))),
              QByteArray::fromHex("0200c0102200011023035942"));

    // Valid C0/03 device acknowledgement, returning the resistance level.
    QByteArray response = kettlerclassicprotocol::encodeFrame(
        0x00c0, 0x03, QByteArray(1, char(6)));
    const auto frames = kettlerclassicprotocol::consumeFrames(response);
    ASSERT_EQ(frames.size(), 1);
    EXPECT_EQ(frames.at(0).function, 0x00c0);
    EXPECT_EQ(frames.at(0).subFunction, 0x03);
    EXPECT_EQ(frames.at(0).payload, QByteArray(1, char(6)));
    EXPECT_TRUE(response.isEmpty());
}

TEST(KettlerClassicProtocol, CoachSE0LastByteReportsResistance) {
    QByteArray capturedTelemetry = QByteArray::fromHex(
        "0004000400000041006f045829e30453002400aa16520001");
    ASSERT_EQ(capturedTelemetry.size(), 24);
    EXPECT_EQ(kettlerclassicprotocol::coachSReportedResistance(capturedTelemetry), 1);
    for (quint8 level : {quint8(3), quint8(4), quint8(5), quint8(6), quint8(10)}) {
        capturedTelemetry[23] = static_cast<char>(level);
        EXPECT_EQ(kettlerclassicprotocol::coachSReportedResistance(capturedTelemetry), level);
    }
    capturedTelemetry[23] = char(0);
    EXPECT_EQ(kettlerclassicprotocol::coachSReportedResistance(capturedTelemetry), 0);
    capturedTelemetry[23] = char(11);
    EXPECT_EQ(kettlerclassicprotocol::coachSReportedResistance(capturedTelemetry), 0);
    EXPECT_EQ(kettlerclassicprotocol::coachSReportedResistance(capturedTelemetry.left(23)), 0);
}

TEST(KettlerClassicProtocol, CoachSPowerPublishesOnlyOnCompletedStrokes) {
    kettlerclassicprotocol::CoachSStrokePowerSmoother smoothing;
    // Old session's 145 W must not leak into a new QZ workout.
    EXPECT_EQ(smoothing.addSample(40, 145, 0), 0);
    EXPECT_EQ(smoothing.addSample(40, 212, 100), 0); // handle nudge
    EXPECT_EQ(smoothing.addSample(40, 231, 300), 0); // no completed stroke
    EXPECT_EQ(smoothing.addSample(40, 145, 500), 0);
    EXPECT_EQ(smoothing.addSample(41, 12, 5200), 12);
    EXPECT_EQ(smoothing.addSample(41, 145, 5400), 12);
    EXPECT_EQ(smoothing.addSample(42, 12, 7200), 12);
    EXPECT_EQ(smoothing.addSample(43, 17, 9200), 17);
    EXPECT_EQ(smoothing.addSample(44, 142, 11200), 142);
    EXPECT_EQ(smoothing.addSample(45, 186, 13200), 186);
    EXPECT_EQ(smoothing.addSample(46, 197, 15200), 197);
    EXPECT_EQ(smoothing.addSample(47, 167, 17200), 167);
}

TEST(KettlerClassicProtocol, CoachSCoastingSamplesCannotKeepPowerAlive) {
    kettlerclassicprotocol::CoachSStrokePowerSmoother smoothing;
    EXPECT_EQ(smoothing.addSample(69, 145, 0), 0);
    EXPECT_EQ(smoothing.addSample(70, 186, 1000), 186);
    EXPECT_EQ(smoothing.addSample(70, 999, 2000), 186);
    EXPECT_EQ(smoothing.addSample(71, 167, 3000), 167);
    EXPECT_EQ(smoothing.addSample(71, 999, 4500), 167);
    EXPECT_TRUE(smoothing.active(6000));
    EXPECT_EQ(smoothing.addSample(71, 999, 6100), 0);
    EXPECT_FALSE(smoothing.active(6100));
    for (int ms = 6500; ms <= 20000; ms += 500)
        EXPECT_EQ(smoothing.addSample(71, 999, ms), 0);
    EXPECT_EQ(smoothing.value(20500), 0);
    EXPECT_EQ(smoothing.addSample(72, 12, 21000), 12);
    EXPECT_TRUE(smoothing.active(21000));
}

TEST(KettlerClassicProtocol, CoachSShortStrokeDoesNotTruncateNextSlowerRecovery) {
    kettlerclassicprotocol::CoachSStrokePowerSmoother smoothing;
    EXPECT_EQ(smoothing.addSample(60, 50, 0), 0);
    EXPECT_EQ(smoothing.addSample(61, 100, 1000), 100);
    EXPECT_EQ(smoothing.addSample(62, 200, 2470), 200);
    // Captured adjacent periods: 1.47 s then 2.57 s. Keep the minimum
    // 3-second inactivity window, not 1.5 seconds.
    EXPECT_EQ(smoothing.value(5030), 200);
    EXPECT_EQ(smoothing.addSample(63, 80, 5040), 80);
}

TEST(KettlerClassicProtocol, CoachSStrokePowerAdaptiveStopAndReset) {
    kettlerclassicprotocol::CoachSStrokePowerSmoother smoothing;
    EXPECT_EQ(smoothing.addSample(1, 160, 0), 0);
    EXPECT_EQ(smoothing.addSample(2, 20, 1000), 20);
    EXPECT_EQ(smoothing.addSample(2, 999, 4000), 20);
    EXPECT_EQ(smoothing.addSample(3, 30, 5000), 30);
    EXPECT_EQ(smoothing.value(10000), 30);
    EXPECT_EQ(smoothing.value(11001), 0);
    EXPECT_EQ(smoothing.addSample(0, 240, 12000), 0);
    EXPECT_EQ(smoothing.addSample(1, 9, 14000), 9);
    smoothing.reset();
    EXPECT_FALSE(smoothing.active(15000));
    EXPECT_EQ(smoothing.value(15000), 0);
}

TEST(KettlerClassicProtocol, CoachSDefaultL4UsesConfirmedC0Write) {
    // Default L4 is the same real Kinomap write proven in the hardware test.
    const QByteArray initialL4 =
        kettlerclassicprotocol::encodeFrameWithEscapedPayloadLength(
            0x00c0, 0x02, QByteArray(1, char(4)));
    EXPECT_EQ(initialL4, QByteArray::fromHex("0200c01022000104032dfd"));
}

TEST(KettlerClassicProtocol, CoachSCounterResetDuringActiveRowClearsPreviousStroke) {
    kettlerclassicprotocol::CoachSStrokePowerSmoother smoothing;
    EXPECT_EQ(smoothing.addSample(20, 120, 0), 0);
    EXPECT_EQ(smoothing.addSample(20, 180, 200), 0);
    EXPECT_EQ(smoothing.addSample(21, 180, 800), 180);
    EXPECT_EQ(smoothing.addSample(0, 300, 1200), 0);
    EXPECT_EQ(smoothing.addSample(0, 300, 1400), 0);
    EXPECT_EQ(smoothing.addSample(1, 10, 2600), 10);
    EXPECT_EQ(smoothing.value(5701), 0);
}

TEST(KettlerClassicProtocol, CoachSSessionCountersStartAtZeroAndRebase) {
    kettlerclassicprotocol::CoachSSessionCounters counters;
    EXPECT_TRUE(counters.addSample(71, 653, false));
    EXPECT_EQ(counters.strokes(), 0u);
    EXPECT_EQ(counters.meters(), 0u);
    EXPECT_FALSE(counters.addSample(71, 660, false)); // idle flywheel
    EXPECT_EQ(counters.meters(), 0u);
    EXPECT_FALSE(counters.addSample(72, 663, true)); // first fresh stroke
    EXPECT_EQ(counters.strokes(), 1u);
    EXPECT_EQ(counters.meters(), 3u);
    EXPECT_FALSE(counters.addSample(72, 666, true)); // no new stroke
    EXPECT_EQ(counters.meters(), 3u);
    EXPECT_FALSE(counters.addSample(72, 671, false)); // idle rebase
    EXPECT_FALSE(counters.addSample(73, 675, true));
    EXPECT_EQ(counters.strokes(), 2u);
    EXPECT_EQ(counters.meters(), 7u);
    counters.clearWorkout();
    EXPECT_EQ(counters.strokes(), 0u);
    EXPECT_EQ(counters.meters(), 0u);
    EXPECT_FALSE(counters.addSample(73, 677, false));
    EXPECT_FALSE(counters.addSample(74, 682, true));
    EXPECT_EQ(counters.strokes(), 1u);
    EXPECT_EQ(counters.meters(), 5u);
    EXPECT_TRUE(counters.addSample(0, 0, false)); // console reset
    EXPECT_EQ(counters.strokes(), 1u);
    EXPECT_EQ(counters.meters(), 5u);
    EXPECT_FALSE(counters.addSample(1, 9, true));
    EXPECT_EQ(counters.strokes(), 2u);
    EXPECT_EQ(counters.meters(), 14u);
    counters.reset();
    EXPECT_TRUE(counters.addSample(71, 653, false));
    EXPECT_EQ(counters.strokes(), 0u);
    EXPECT_EQ(counters.meters(), 0u);
}

TEST(KettlerClassicProtocol, CoachSConcept2PaceUsesWattsNotFlywheelDistance) {
    kettlerclassicprotocol::CoachSPowerBasedPace pace;
    // 2026-11-18, 20:35:50 (L4, 185 W) and 20:35:58 (L5, 338 W):
    // raw Coach S flywheel distance remained about 8 m/stroke despite
    // almost doubled real power. Concept2-equivalent speed must increase.
    pace.completedStroke(185, -1, false); // first stroke: no interval yet
    const double lowSpeed = pace.speedKmh();
    EXPECT_NEAR(lowSpeed, std::cbrt(185.0 / 2.8) * 3.6, 0.001);
    EXPECT_NEAR(500.0 / (lowSpeed / 3.6), 123.7, 0.5); // s/500 m
    EXPECT_DOUBLE_EQ(pace.meters(), 0.0);

    pace.completedStroke(185, 1981, true);
    const double firstDistance = pace.meters();
    EXPECT_NEAR(firstDistance, (lowSpeed / 3.6) * 1.981, 0.001);

    pace.completedStroke(338, 1981, true);
    const double highSpeed = pace.speedKmh();
    EXPECT_NEAR(highSpeed, std::cbrt(338.0 / 2.8) * 3.6, 0.001);
    EXPECT_NEAR(500.0 / (highSpeed / 3.6), 101.2, 0.5); // s/500 m
    EXPECT_GT(highSpeed, lowSpeed);
    EXPECT_NEAR(pace.meters() - firstDistance,
                (highSpeed / 3.6) * 1.981, 0.001);
}

TEST(KettlerClassicProtocol, CoachSConcept2NoGhostMetersAndStopsCleanly) {
    kettlerclassicprotocol::CoachSPowerBasedPace pace;
    pace.completedStroke(145, -1, false); // old initial console watt is ignored
    // A fresh QZ session has no committed distance before a second stroke.
    EXPECT_DOUBLE_EQ(pace.meters(), 0.0);
    pace.reset();
    EXPECT_DOUBLE_EQ(pace.speedKmh(), 0.0);
    pace.completedStroke(12, -1, false);
    EXPECT_DOUBLE_EQ(pace.meters(), 0.0);
    pace.completedStroke(17, 2000, true);
    const double lastStrokeDistance = pace.meters();
    EXPECT_GT(lastStrokeDistance, 0.0);
    // Idle E0 telemetry from a coasting flywheel does not invoke
    // completedStroke(): both speed and session distance stay unchanged
    // until the caller's existing adaptive timeout zeros the speed.
    EXPECT_DOUBLE_EQ(pace.meters(), lastStrokeDistance);

    pace.completedStroke(142, 8000, false); // first stroke after long stop
    EXPECT_DOUBLE_EQ(pace.meters(), lastStrokeDistance);
    EXPECT_GT(pace.speedKmh(), 0.0);
    pace.completedStroke(186, 2000, true);
    EXPECT_GT(pace.meters(), lastStrokeDistance);

    const double latest = pace.meters();
    pace.completedStroke(0, 2000, true);
    EXPECT_DOUBLE_EQ(pace.speedKmh(), 0.0);
    EXPECT_DOUBLE_EQ(pace.meters(), latest);
    pace.reset();
    EXPECT_DOUBLE_EQ(pace.speedKmh(), 0.0);
    EXPECT_DOUBLE_EQ(pace.meters(), 0.0);
}

TEST(KettlerClassicProtocol, CoachSConcept2ModeTogglePreservesDistance) {
    kettlerclassicprotocol::CoachSPowerBasedPace pace;
    pace.reset(87.5); // existing raw-counter workout when setting enabled
    pace.completedStroke(185, -1, false);
    EXPECT_DOUBLE_EQ(pace.meters(), 87.5);
    pace.completedStroke(185, 2000, true);
    EXPECT_GT(pace.meters(), 87.5);
    const double beforeSwitch = pace.meters();
    pace.reset(beforeSwitch); // re-enable power-based mode mid-session
    EXPECT_DOUBLE_EQ(pace.meters(), beforeSwitch);
    EXPECT_DOUBLE_EQ(pace.speedKmh(), 0.0);
    pace.completedStroke(185, 2000, false);
    EXPECT_DOUBLE_EQ(pace.meters(), beforeSwitch);
}

TEST(KettlerClassicProtocol, CoachSNovember18NudgeAndCoastCannotCreditDistance) {
    kettlerclassicprotocol::CoachSSessionCounters counters;
    EXPECT_TRUE(counters.addSample(40, 430, false));
    for (quint16 meters = 431; meters <= 439; ++meters)
        EXPECT_FALSE(counters.addSample(40, meters, false));
    EXPECT_EQ(counters.strokes(), 0u);
    EXPECT_EQ(counters.meters(), 0u);
    EXPECT_FALSE(counters.addSample(41, 440, true));
    EXPECT_EQ(counters.meters(), 1u);
    EXPECT_FALSE(counters.addSample(41, 449, true));
    EXPECT_EQ(counters.meters(), 1u);
    EXPECT_FALSE(counters.addSample(41, 450, false));
    EXPECT_FALSE(counters.addSample(42, 450, true));
    EXPECT_EQ(counters.meters(), 1u);
    EXPECT_FALSE(counters.addSample(42, 463, false));
    EXPECT_FALSE(counters.addSample(43, 464, true));
    EXPECT_FALSE(counters.addSample(44, 474, true));
    EXPECT_FALSE(counters.addSample(45, 480, true));
    EXPECT_FALSE(counters.addSample(46, 487, true));
    EXPECT_FALSE(counters.addSample(47, 495, true));
    const quint64 distanceAtStop = counters.meters();
    EXPECT_FALSE(counters.addSample(47, 507, true));
    EXPECT_FALSE(counters.addSample(47, 518, false));
    EXPECT_EQ(counters.strokes(), 7u);
    EXPECT_EQ(counters.meters(), distanceAtStop);
    counters.reset();
    EXPECT_TRUE(counters.addSample(47, 518, false));
    EXPECT_EQ(counters.strokes(), 0u);
    EXPECT_EQ(counters.meters(), 0u);
}
