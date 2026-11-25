#include "devices/horizontreadmill/paragonxtelemetry.h"
#include "devices/horizontreadmill/horizontreadmill.h"
#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// Linux CI runs from tst/, while local developers may launch the test
// binary from the repository root or from a build directory.
static std::string fixturePath(const char *filename) {
    for (const char *prefix : {"../tests/fixtures/", "tests/fixtures/", "../../tests/fixtures/"}) {
        const std::string path = std::string(prefix) + filename;
        if (std::ifstream(path))
            return path;
    }
    throw std::runtime_error(std::string("Paragon X BLE fixture not found: ") + filename);
}

static std::vector<uint8_t> parseHex(const std::string &text) {
    if (text.size() % 2)
        throw std::runtime_error("odd length hex fragment");
    std::vector<uint8_t> bytes;
    for (std::size_t i = 0; i < text.size(); i += 2)
        bytes.push_back(static_cast<uint8_t>(std::stoul(text.substr(i, 2), nullptr, 16)));
    return bytes;
}

static void testFixtures(const std::string &path, bool incomplete) {
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("cannot open " + path);
    int samples = 0;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream fields(line);
        std::string fragments, speedString, inclineString;
        if (!std::getline(fields, fragments, '\t') || !std::getline(fields, speedString, '\t') ||
            !std::getline(fields, inclineString))
            throw std::runtime_error("invalid fixture row");
        const double expectedSpeed = std::stod(speedString);
        const double expectedIncline = std::stod(inclineString);
        std::vector<uint8_t> frame;
        std::istringstream pieces(fragments);
        std::string piece;
        int chunks = 0;
        while (std::getline(pieces, piece, ',')) {
            auto bytes = parseHex(piece);
            frame.insert(frame.end(), bytes.begin(), bytes.end());
            ++chunks;
            // A fragmented status packet must never be interpreted as a
            // complete status update before the expected last fragment.
            if (incomplete || pieces.peek() != EOF) {
                ParagonXTelemetry::Metrics discarded{};
                if (ParagonXTelemetry::decode(frame.data(), frame.size(), false, discarded))
                    throw std::runtime_error("premature decode of partial frame");
            }
        }
        if (incomplete && (chunks != 3 || frame.size() != 60))
            throw std::runtime_error("unexpected truncated frame layout");
        if (!incomplete && frame.size() != 76)
            throw std::runtime_error("unexpected complete frame layout");
        ParagonXTelemetry::Metrics metrics{};
        if (!ParagonXTelemetry::decode(frame.data(), frame.size(), incomplete, metrics))
            throw std::runtime_error("failed to decode recorded telemetry");
        if (std::abs(metrics.speedKmh - expectedSpeed) > 1e-5 ||
            std::abs(metrics.inclinePercent - expectedIncline) > 1e-5)
            throw std::runtime_error("decoded telemetry differs from captured fixture");
        if (incomplete && ParagonXTelemetry::decode(frame.data(), frame.size(), false, metrics))
            throw std::runtime_error("truncated frame accepted without explicit boundary");
        if (incomplete) {
            // The application must only promote a truncated frame when the
            // next 0x55aa header arrives while bytes are still outstanding.
            const uint8_t nextFrameHeader[] = {0x55, 0xaa, 0, 0, 2, 0x17, 0x40, 0};
            const uint8_t continuation[] = {0, 0, 0, 0, 0, 0, 0, 0};
            ParagonXTelemetry::Metrics boundary{};
            if (!ParagonXTelemetry::decodePreviousAtBoundary(frame.data(), frame.size(), 14,
                                                              nextFrameHeader, sizeof(nextFrameHeader), boundary) ||
                std::abs(boundary.speedKmh - expectedSpeed) > 1e-5)
                throw std::runtime_error("valid truncated frame lost at next packet boundary");
            if (ParagonXTelemetry::decodePreviousAtBoundary(frame.data(), frame.size(), 0,
                                                            nextFrameHeader, sizeof(nextFrameHeader), boundary) ||
                ParagonXTelemetry::decodePreviousAtBoundary(frame.data(), frame.size(), 14,
                                                            continuation, sizeof(continuation), boundary))
                throw std::runtime_error("truncated frame decoded without valid boundary");
            // Missing last fragment: reject if one of three redundant speed
            // copies disagrees (we cannot check the end-of-frame checksum).
            frame[26] ^= 0x01;
            if (ParagonXTelemetry::decode(frame.data(), frame.size(), true, metrics))
                throw std::runtime_error("inconsistent truncated speed accepted");
        }
        ++samples;
    }
    if (samples < 10)
        throw std::runtime_error("too few real packets in fixture " + path);
    std::cout << path << ": " << samples << " captured frames OK\n";
}

// These tests are part of the regular qdomyos-zwift-tests GoogleTest executable.
// The existing CI job runs the full suite on EVERY pull request and on master,
// regardless of which source files were changed.
TEST(ParagonXTelemetryTest, ExistingCompleteFramesFromOctober2026) {
    EXPECT_NO_THROW(testFixtures(fixturePath("paragonx_complete_2026.tsv"), false));
}

TEST(ParagonXTelemetryTest, TruncatedFramesFromNovember2026) {
    EXPECT_NO_THROW(testFixtures(fixturePath("paragonx_truncated_2026.tsv"), true));
}

TEST(ParagonXTelemetryTest, RejectsUnrelatedPackets) {
    const uint8_t notTelemetry[] = {0x55, 0xaa, 0, 0, 2, 0x20, 0x40, 0};
    ParagonXTelemetry::Metrics metrics{};
    EXPECT_FALSE(ParagonXTelemetry::decode(notTelemetry, sizeof(notTelemetry), true, metrics));
}


namespace {
struct CapturedFrame {
    std::vector<std::vector<uint8_t>> notifications;
    double speedKmh;
    double inclinePercent;
};

std::vector<CapturedFrame> readCapturedFrames(const std::string &path) {
    std::ifstream input(path);
    if (!input)
        throw std::runtime_error("cannot open BLE capture " + path);
    std::vector<CapturedFrame> frames;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream fields(line);
        std::string fragments, speedString, inclineString;
        if (!std::getline(fields, fragments, '\t') ||
            !std::getline(fields, speedString, '\t') ||
            !std::getline(fields, inclineString))
            throw std::runtime_error("malformed BLE capture row");
        CapturedFrame frame{};
        frame.speedKmh = std::stod(speedString);
        frame.inclinePercent = std::stod(inclineString);
        std::istringstream fragmentsStream(fragments);
        std::string hex;
        while (std::getline(fragmentsStream, hex, ','))
            frame.notifications.push_back(parseHex(hex));
        frames.push_back(frame);
    }
    return frames;
}
} // namespace

TEST(ParagonXTelemetryTest, CompleteBLEStreamSignalsOncePerFrame) {
    const auto frames = readCapturedFrames(fixturePath("paragonx_complete_2026.tsv"));
    ASSERT_EQ(frames.size(), 19u);
    ParagonXTelemetry::NotificationAssembler assembler;
    unsigned packetReceivedSignals = 0;
    unsigned telemetryUpdates = 0;

    for (const auto &capture : frames) {
        for (std::size_t i = 0; i < capture.notifications.size(); ++i) {
            const auto &notification = capture.notifications[i];
            const auto result = assembler.accept(
                notification.data(), notification.size(), [&]() { ++packetReceivedSignals; });
            EXPECT_FALSE(result.recovered);
            if (i + 1 < capture.notifications.size()) {
                EXPECT_FALSE(result.completed);
            } else {
                EXPECT_TRUE(result.completed);
                const auto &packet = assembler.frame();
                ParagonXTelemetry::Metrics metrics{};
                ASSERT_TRUE(ParagonXTelemetry::decode(packet.data(), packet.size(), false, metrics));
                EXPECT_NEAR(metrics.speedKmh, capture.speedKmh, 0.00001);
                EXPECT_NEAR(metrics.inclinePercent, capture.inclinePercent, 0.00001);
                ++telemetryUpdates;
            }
        }
        // Completed frame must not cause multiple packetReceived() emissions.
        EXPECT_EQ(packetReceivedSignals, telemetryUpdates);
    }
    EXPECT_EQ(packetReceivedSignals, frames.size());
    EXPECT_EQ(telemetryUpdates, frames.size());
}

TEST(ParagonXTelemetryTest, IncompleteBLEStreamSignalsOnNextFrameBoundary) {
    const auto frames = readCapturedFrames(fixturePath("paragonx_truncated_2026.tsv"));
    ASSERT_EQ(frames.size(), 16u);
    ParagonXTelemetry::NotificationAssembler assembler;
    unsigned packetReceivedSignals = 0;
    unsigned telemetryUpdates = 0;
    for (std::size_t frameIndex = 0; frameIndex < frames.size(); ++frameIndex) {
        const auto &capture = frames[frameIndex];
        ASSERT_EQ(capture.notifications.size(), 3u);
        for (std::size_t chunkIndex = 0; chunkIndex < capture.notifications.size(); ++chunkIndex) {
            const auto &notification = capture.notifications[chunkIndex];
            const auto result = assembler.accept(
                notification.data(), notification.size(), [&]() { ++packetReceivedSignals; });
            EXPECT_FALSE(result.completed); // 60 bytes never satisfies declared 74.
            if (frameIndex && chunkIndex == 0) {
                const auto &previous = frames[frameIndex - 1];
                ASSERT_TRUE(result.recovered); // previous frame is complete enough at this boundary
                EXPECT_NEAR(result.recoveredMetrics.speedKmh, previous.speedKmh, 0.00001);
                EXPECT_NEAR(result.recoveredMetrics.inclinePercent, previous.inclinePercent, 0.00001);
                ++telemetryUpdates;
            } else {
                EXPECT_FALSE(result.recovered);
            }
            EXPECT_EQ(packetReceivedSignals, telemetryUpdates);
        }
    }
    // One last boundary is required to conclude that the final 60-byte packet
    // will not receive more BLE fragments (without it, no update is emitted).
    const auto &nextHeader = frames.front().notifications.front();
    const auto final = assembler.accept(nextHeader.data(), nextHeader.size(),
                                        [&]() { ++packetReceivedSignals; });
    ASSERT_TRUE(final.recovered);
    EXPECT_FALSE(final.completed);
    EXPECT_NEAR(final.recoveredMetrics.speedKmh, frames.back().speedKmh, 0.00001);
    EXPECT_NEAR(final.recoveredMetrics.inclinePercent, frames.back().inclinePercent, 0.00001);
    ++telemetryUpdates;
    EXPECT_EQ(packetReceivedSignals, telemetryUpdates);
    EXPECT_EQ(telemetryUpdates, frames.size());
}

TEST(ParagonXTelemetryTest, PartialOrCorruptBLEFramesNeverEmitPacketReceived) {
    const auto frames = readCapturedFrames(fixturePath("paragonx_truncated_2026.tsv"));
    ASSERT_FALSE(frames.empty());
    const auto &chunks = frames.front().notifications;
    const auto &nextHeader = frames[1].notifications.front();
    unsigned packetReceivedSignals = 0;
    ParagonXTelemetry::NotificationAssembler assembler;
    for (const auto &chunk : chunks) {
        const auto result = assembler.accept(chunk.data(), chunk.size(),
                                              [&]() { ++packetReceivedSignals; });
        EXPECT_FALSE(result.completed);
        EXPECT_FALSE(result.recovered);
        EXPECT_EQ(packetReceivedSignals, 0u);
    }
    assembler.reset();
    for (std::size_t i = 0; i < chunks.size(); ++i) {
        auto chunk = chunks[i];
        if (i == 1)
            chunk[6] ^= 0x01; // corrupt the second redundant speed copy
        assembler.accept(chunk.data(), chunk.size(), [&]() { ++packetReceivedSignals; });
    }
    const auto invalid = assembler.accept(nextHeader.data(), nextHeader.size(),
                                           [&]() { ++packetReceivedSignals; });
    EXPECT_FALSE(invalid.recovered);
    EXPECT_FALSE(invalid.completed);
    EXPECT_EQ(packetReceivedSignals, 0u);
}

TEST(ParagonXTelemetryTest, InitRepliesStillEmitPacketReceivedOnce) {
    ParagonXTelemetry::NotificationAssembler assembler;
    unsigned packetReceivedSignals = 0;
    // Non-telemetry initialization reply: same declared-length assembly
    // as the original Horizon driver and the same packetReceived() callback.
    const std::vector<uint8_t> reply = {0x55, 0xaa, 0, 0, 2, 0x20, 2, 0, 0, 0, 0x0d, 0x0a};
    const auto first = assembler.accept(reply.data(), 8, [&]() { ++packetReceivedSignals; });
    EXPECT_FALSE(first.completed);
    EXPECT_EQ(packetReceivedSignals, 0u);
    const auto last = assembler.accept(reply.data() + 8, 4, [&]() { ++packetReceivedSignals; });
    EXPECT_TRUE(last.completed);
    EXPECT_FALSE(last.recovered);
    EXPECT_EQ(packetReceivedSignals, 1u);
    // The original driver also signaled unframed short init replies; keep
    // this behavior to avoid breaking handshakes with older Paragon X units.
    const uint8_t headerlessReply[] = {0x01, 0, 0, 0, 0, 0, 0x0d, 0x0a};
    const auto standalone = assembler.accept(headerlessReply, sizeof(headerlessReply),
                                             [&]() { ++packetReceivedSignals; });
    EXPECT_TRUE(standalone.completed);
    EXPECT_EQ(packetReceivedSignals, 2u);
}


// Only the test can inject BLE notifications directly into the production
// receiver. This tests the *actual Qt packetReceived signal* rather than a
// substitute callback or a duplicate implementation of the frame assembler.
class ParagonXNotificationTestAccess {
  public:
    static void applyTelemetry(horizontreadmill &driver, const ParagonXTelemetry::Metrics &metrics) {
        driver.applyParagonTelemetry(metrics);
    }
    static void stopRequested(horizontreadmill &driver) { driver.paragonSessionState.stopRequested(); }
    static void startRequested(horizontreadmill &driver) { driver.paragonSessionState.startRequested(); }
    static bool commandsSuspended(const horizontreadmill &driver) {
        return driver.paragonSessionState.commandsSuspended();
    }
    static ParagonXTelemetry::NotificationResult feed(horizontreadmill &driver,
                                                      const std::vector<uint8_t> &bytes) {
        return driver.processParagonNotification(
            QByteArray(reinterpret_cast<const char *>(bytes.data()), static_cast<int>(bytes.size())));
    }
    static bool didReceiveInitResponse(const horizontreadmill &driver) {
        return driver.initPacketRecv;
    }
};

TEST(ParagonXTelemetryTest, ProductionDriverSignalsOnceForEveryCompleteFrame) {
    const auto captures = readCapturedFrames(fixturePath("paragonx_complete_2026.tsv"));
    ASSERT_EQ(captures.size(), 19u);
    horizontreadmill driver(false, false);
    unsigned signalCount = 0;
    QObject::connect(&driver, &horizontreadmill::packetReceived, &driver, [&]() { ++signalCount; });
    for (std::size_t frameIndex = 0; frameIndex < captures.size(); ++frameIndex) {
        const auto &chunks = captures[frameIndex].notifications;
        for (std::size_t i = 0; i < chunks.size(); ++i) {
            const auto result = ParagonXNotificationTestAccess::feed(driver, chunks[i]);
            EXPECT_FALSE(result.recovered);
            EXPECT_EQ(result.completed, i + 1 == chunks.size());
            EXPECT_EQ(signalCount, frameIndex + (i + 1 == chunks.size() ? 1u : 0u));
        }
    }
    EXPECT_EQ(signalCount, captures.size());
    EXPECT_TRUE(ParagonXNotificationTestAccess::didReceiveInitResponse(driver));
}

TEST(ParagonXTelemetryTest, ProductionDriverSignalsOnTruncatedFrameBoundary) {
    const auto captures = readCapturedFrames(fixturePath("paragonx_truncated_2026.tsv"));
    ASSERT_EQ(captures.size(), 16u);
    horizontreadmill driver(false, false);
    unsigned signalCount = 0;
    QObject::connect(&driver, &horizontreadmill::packetReceived, &driver, [&]() { ++signalCount; });
    for (std::size_t frameIndex = 0; frameIndex < captures.size(); ++frameIndex) {
        const auto &chunks = captures[frameIndex].notifications;
        for (std::size_t i = 0; i < chunks.size(); ++i) {
            const auto result = ParagonXNotificationTestAccess::feed(driver, chunks[i]);
            EXPECT_FALSE(result.completed);
            if (frameIndex && i == 0) {
                EXPECT_TRUE(result.recovered);
                EXPECT_NEAR(result.recoveredMetrics.speedKmh, captures[frameIndex - 1].speedKmh, 0.00001);
                EXPECT_NEAR(result.recoveredMetrics.inclinePercent, captures[frameIndex - 1].inclinePercent, 0.00001);
            } else {
                EXPECT_FALSE(result.recovered);
            }
            EXPECT_EQ(signalCount, frameIndex);
        }
    }
    // Final recovery requires another packet boundary.
    EXPECT_EQ(signalCount, captures.size() - 1);
    const auto final = ParagonXNotificationTestAccess::feed(driver, captures.front().notifications.front());
    EXPECT_TRUE(final.recovered);
    EXPECT_FALSE(final.completed);
    EXPECT_EQ(signalCount, captures.size());
    EXPECT_TRUE(ParagonXNotificationTestAccess::didReceiveInitResponse(driver));
}

TEST(ParagonXTelemetryTest, ProductionDriverPreservesInitReplySignal) {
    horizontreadmill driver(false, false);
    unsigned signalCount = 0;
    QObject::connect(&driver, &horizontreadmill::packetReceived, &driver, [&]() { ++signalCount; });
    const std::vector<uint8_t> first = {0x55, 0xaa, 0, 0, 2, 0x20, 2, 0};
    const std::vector<uint8_t> last = {0, 0, 0x0d, 0x0a};
    const auto partial = ParagonXNotificationTestAccess::feed(driver, first);
    EXPECT_FALSE(partial.completed);
    EXPECT_EQ(signalCount, 0u);
    const auto full = ParagonXNotificationTestAccess::feed(driver, last);
    EXPECT_TRUE(full.completed);
    EXPECT_EQ(signalCount, 1u);
    // Preserve headerless initialization replies as well.
    const std::vector<uint8_t> standalone = {0x01, 0, 0, 0, 0, 0, 0x0d, 0x0a};
    const auto reply = ParagonXNotificationTestAccess::feed(driver, standalone);
    EXPECT_TRUE(reply.completed);
    EXPECT_EQ(signalCount, 2u);
}

namespace {
std::vector<ParagonXTelemetry::Metrics> readActualParagonMotionStates() {
    const auto captures = readCapturedFrames(fixturePath("paragonx_pause_stop_2026.tsv"));
    if (captures.size() != 3)
        throw std::runtime_error("expected real running, paused, stopped captures");
    std::vector<ParagonXTelemetry::Metrics> metrics;
    for (const auto &capture : captures) {
        std::vector<uint8_t> bytes;
        for (const auto &chunk : capture.notifications)
            bytes.insert(bytes.end(), chunk.begin(), chunk.end());
        if (bytes.size() != 60)
            throw std::runtime_error("real Paragon pause/stop capture must contain 60 bytes");
        ParagonXTelemetry::Metrics decoded{};
        if (!ParagonXTelemetry::decode(bytes.data(), bytes.size(), true, decoded))
            throw std::runtime_error("failed to decode real Paragon motion-state frame");
        if (std::abs(decoded.speedKmh - capture.speedKmh) > 0.00001 ||
            std::abs(decoded.inclinePercent - capture.inclinePercent) > 0.00001)
            throw std::runtime_error("motion-state raw fields differ from captured values");
        metrics.push_back(decoded);
    }
    return metrics;
}
} // namespace

TEST(ParagonXTelemetryTest, ActualRunningPauseStopFramesSuppressStaleSpeed) {
    const auto frames = readActualParagonMotionStates();
    ASSERT_EQ(frames.size(), 3u);
    EXPECT_EQ(frames[0].machineState, 0x01);
    EXPECT_EQ(frames[1].machineState, 0x02);
    EXPECT_EQ(frames[2].machineState, 0x04);
    // The raw speed is still nonzero in the actual pause AND stop packets.
    EXPECT_GT(frames[1].speedKmh, 8.0);
    EXPECT_GT(frames[2].speedKmh, 11.0);

    ParagonXTelemetry::SessionState session;
    session.observe(frames[0].machineState);
    EXPECT_FALSE(session.commandsSuspended());
    EXPECT_NEAR(session.publishedSpeed(frames[0].speedKmh), 8.80309, 0.00001);

    // QZ's Stop/Pause button must zero virtual speed immediately, before the
    // treadmill has time to change its machine status to 0x02/0x04.
    session.stopRequested();
    EXPECT_TRUE(session.commandsSuspended());
    EXPECT_EQ(session.publishedSpeed(frames[0].speedKmh), 0.0);
    session.observe(frames[0].machineState);
    EXPECT_EQ(session.publishedSpeed(frames[0].speedKmh), 0.0);

    session.observe(frames[1].machineState); // physical Pause status
    EXPECT_TRUE(session.commandsSuspended());
    EXPECT_EQ(session.publishedSpeed(frames[1].speedKmh), 0.0);
    session.observe(frames[2].machineState); // physical Stop status
    EXPECT_EQ(session.publishedSpeed(frames[2].speedKmh), 0.0);

    // Restart with the physical console (0x04 -> 0x01) must release the latch.
    session.observe(frames[0].machineState);
    EXPECT_FALSE(session.commandsSuspended());
    EXPECT_NEAR(session.publishedSpeed(frames[0].speedKmh), 8.80309, 0.00001);

    // Restart through QZ must work even when the previous status was running.
    session.stopRequested();
    EXPECT_EQ(session.publishedSpeed(frames[0].speedKmh), 0.0);
    session.startRequested();
    session.observe(frames[0].machineState);
    EXPECT_FALSE(session.commandsSuspended());
    EXPECT_GT(session.publishedSpeed(frames[0].speedKmh), 8.0);

    // Unknown firmware states must not silently be interpreted as stopped.
    session.observe(0x7f);
    EXPECT_FALSE(session.commandsSuspended());
}

TEST(ParagonXTelemetryTest, ProductionDriverPublishesZeroAfterPauseAndStop) {
    const auto frames = readActualParagonMotionStates();
    ASSERT_EQ(frames.size(), 3u);
    horizontreadmill driver(false, false);
    ParagonXNotificationTestAccess::applyTelemetry(driver, frames[0]);
    EXPECT_NEAR(driver.currentSpeed().value(), 8.80309, 0.00001);
    ParagonXNotificationTestAccess::stopRequested(driver);
    EXPECT_TRUE(ParagonXNotificationTestAccess::commandsSuspended(driver));

    // The console may continue sending running status for a moment.
    ParagonXNotificationTestAccess::applyTelemetry(driver, frames[0]);
    EXPECT_DOUBLE_EQ(driver.currentSpeed().value(), 0.0);
    ParagonXNotificationTestAccess::applyTelemetry(driver, frames[1]);
    EXPECT_DOUBLE_EQ(driver.currentSpeed().value(), 0.0);
    ParagonXNotificationTestAccess::applyTelemetry(driver, frames[2]);
    EXPECT_DOUBLE_EQ(driver.currentSpeed().value(), 0.0);

    // Real transition from pause/stop to running re-enables normal telemetry.
    ParagonXNotificationTestAccess::applyTelemetry(driver, frames[0]);
    EXPECT_NEAR(driver.currentSpeed().value(), 8.80309, 0.00001);

    ParagonXNotificationTestAccess::stopRequested(driver);
    ParagonXNotificationTestAccess::startRequested(driver);
    ParagonXNotificationTestAccess::applyTelemetry(driver, frames[0]);
    EXPECT_NEAR(driver.currentSpeed().value(), 8.80309, 0.00001);
}


TEST(ParagonXTelemetryTest, ExistingCompleteFrameFirmwareStillReportsRunningAndPause) {
    // Vincenzo's known-good Paragon X sends 76-byte status frames. The final
    // frames retain the last raw running speed, but machine state becomes 0x02.
    // Preserve exact raw decoding while publishing zero for paused status,
    // matching the shorter-frame variant and preventing Zwift ghost running.
    const auto captures = readCapturedFrames(fixturePath("paragonx_complete_2026.tsv"));
    ASSERT_EQ(captures.size(), 19u);
    horizontreadmill driver(false, false);
    unsigned runningFrames = 0;
    unsigned pausedFrames = 0;
    for (const auto &capture : captures) {
        std::vector<uint8_t> frame;
        for (const auto &chunk : capture.notifications)
            frame.insert(frame.end(), chunk.begin(), chunk.end());
        ASSERT_EQ(frame.size(), 76u);
        ParagonXTelemetry::Metrics metrics{};
        ASSERT_TRUE(ParagonXTelemetry::decode(frame.data(), frame.size(), false, metrics));
        EXPECT_NEAR(metrics.speedKmh, capture.speedKmh, 0.00001);
        ParagonXNotificationTestAccess::applyTelemetry(driver, metrics);
        if (metrics.machineState == 0x01) {
            ++runningFrames;
            EXPECT_NEAR(driver.currentSpeed().value(), capture.speedKmh, 0.00001);
        } else if (metrics.machineState == 0x02) {
            ++pausedFrames;
            EXPECT_DOUBLE_EQ(driver.currentSpeed().value(), 0.0);
            EXPECT_TRUE(ParagonXNotificationTestAccess::commandsSuspended(driver));
        } else {
            FAIL() << "Unexpected machine state in the known-good 76-byte capture: "
                   << static_cast<unsigned>(metrics.machineState);
        }
    }
    EXPECT_EQ(runningFrames, 13u);
    EXPECT_EQ(pausedFrames, 6u);
}
