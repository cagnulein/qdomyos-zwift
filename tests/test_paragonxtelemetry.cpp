#include "../src/devices/horizontreadmill/paragonxtelemetry.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "usage: test_paragonxtelemetry complete.tsv incomplete.tsv\n";
        return 2;
    }
    try {
        testFixtures(argv[1], false);
        testFixtures(argv[2], true);
        const uint8_t notTelemetry[] = {0x55, 0xaa, 0, 0, 2, 0x20, 0x40, 0};
        ParagonXTelemetry::Metrics metrics{};
        if (ParagonXTelemetry::decode(notTelemetry, sizeof(notTelemetry), true, metrics))
            throw std::runtime_error("non-telemetry decoded");
    } catch (const std::exception &ex) {
        std::cerr << "FAIL: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
