#include "devices/horizontreadmill/paragonxtelemetry.h"
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
    // Do not repeat signals for an unrelated/orphaned continuation fragment.
    const uint8_t orphan[] = {0, 0, 0};
    const auto ignored = assembler.accept(orphan, sizeof(orphan), [&]() { ++packetReceivedSignals; });
    EXPECT_FALSE(ignored.completed);
    EXPECT_EQ(packetReceivedSignals, 1u);
}
