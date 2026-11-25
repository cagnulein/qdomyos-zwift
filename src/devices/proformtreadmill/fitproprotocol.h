// SPDX-License-Identifier: GPL-3.0-or-later
// Unlock algorithm adapted from OpenFit (BaconWappedBitcoin/openfit),
// android/protocol/src/main/kotlin/com/ofit/protocol/UnlockCalculator.kt,
// GPL-3.0-or-later. See docs/fitpro-protocol.md for attribution and evidence.
#ifndef FITPROPROTOCOL_H
#define FITPROPROTOCOL_H

#include <cstdint>
#include <cstddef>
#include <vector>

namespace FitProProtocol {
using Bytes = std::vector<uint8_t>;
using Chunks = std::vector<Bytes>;

inline uint8_t checksum(const Bytes &frame, std::size_t count) {
    uint8_t sum = 0;
    for (std::size_t i = 0; i < count && i < frame.size(); ++i)
        sum = static_cast<uint8_t>(sum + frame[i]);
    return sum;
}

inline bool validFrame(const Bytes &frame) {
    return frame.size() >= 4 && frame.size() <= 255 &&
           frame[1] == frame.size() &&
           frame.back() == checksum(frame, frame.size() - 1);
}

inline Bytes command(uint8_t device, uint8_t opcode, const Bytes &content = {}) {
    if (content.size() > 251)
        return {};
    Bytes frame{device, static_cast<uint8_t>(content.size() + 4), opcode};
    frame.insert(frame.end(), content.begin(), content.end());
    frame.push_back(checksum(frame, frame.size()));
    return frame;
}

// FE header counts itself. Only the declared bytes in each data chunk
// are meaningful: real consoles leave stale bytes in the padding.
inline Chunks encode(const Bytes &frame) {
    if (!validFrame(frame) || frame.size() > 251)
        return {};
    Bytes payload{0x02, 0x04, 0x02, static_cast<uint8_t>(frame.size())};
    payload.insert(payload.end(), frame.begin(), frame.end());
    const std::size_t count = (payload.size() + 17) / 18;
    Chunks result{{0xfe, 0x02, static_cast<uint8_t>(payload.size()),
                   static_cast<uint8_t>(count + 1)}};
    for (std::size_t i = 0, pos = 0; pos < payload.size(); ++i) {
        const std::size_t remaining = payload.size() - pos;
        const std::size_t n = remaining > 18 ? 18 : remaining;
        Bytes chunk{static_cast<uint8_t>(remaining <= 18 ? 0xff : i),
                    static_cast<uint8_t>(n)};
        chunk.insert(chunk.end(), payload.begin() + pos, payload.begin() + pos + n);
        chunk.resize(20, 0);
        result.push_back(chunk);
        pos += n;
    }
    return result;
}

// Reassemble one complete group of BLE notifications (not a streaming
// receiver). Reject incomplete, out-of-order or corrupt groups.
inline Bytes decodeReply(const Chunks &chunks) {
    if (chunks.size() < 2 || chunks.size() > 16)
        return {};
    const Bytes &header = chunks.front();
    if (header.size() < 4 || header.size() > 20 || header[0] != 0xfe ||
        header[1] != 0x02 || header[3] != chunks.size())
        return {};
    Bytes payload;
    for (std::size_t i = 1; i < chunks.size(); ++i) {
        const Bytes &chunk = chunks[i];
        const bool last = i + 1 == chunks.size();
        if (chunk.size() < 2 || chunk.size() > 20 ||
            chunk[0] != (last ? 0xff : i - 1) || chunk[1] == 0 ||
            chunk[1] > 18 || (!last && chunk[1] != 18) ||
            chunk.size() < std::size_t(2 + chunk[1]) ||
            payload.size() + chunk[1] > header[2])
            return {};
        payload.insert(payload.end(), chunk.begin() + 2, chunk.begin() + 2 + chunk[1]);
    }
    if (payload.size() != header[2] || payload.size() < 8 ||
        payload[0] != 0x01 || payload[1] != 0x04 || payload[2] != 0x02 ||
        payload[3] != payload.size() - 4)
        return {};
    Bytes frame(payload.begin() + 4, payload.end());
    return validFrame(frame) ? frame : Bytes{};
}

inline uint32_t littleEndian(const Bytes &bytes, std::size_t offset) {
    uint32_t result = 0;
    for (unsigned i = 0; i < 4; ++i)
        result |= uint32_t(bytes[offset + i]) << (8 * i);
    return result;
}

inline uint32_t signedByte(uint8_t byte) {
    return byte < 128 ? uint32_t(byte) : uint32_t(byte) - 256u;
}

// Explicit arithmetic right shift, matching Kotlin Int without relying
// on implementation-defined signed shifts or signed overflow in C++11.
inline uint32_t arithmeticShift(uint32_t value, unsigned n) {
    if (n == 0)
        return value;
    return (value >> n) | ((value & 0x80000000u) ? (~uint32_t(0) << (32 - n)) : 0u);
}

inline bool successfulReply(const Bytes &frame, uint8_t opcode, std::size_t minSize) {
    return frame.size() >= minSize && validFrame(frame) &&
           frame[0] == 0x04 && frame[2] == opcode && frame[3] == 0x02;
}

// Candidate only: matching a known capture does not establish support for
// other firmware. Never send this automatically from the legacy driver.
inline Bytes computeUnlock(const Bytes &deviceInfo, const Bytes &systemInfo,
                           const Bytes &versionInfo) {
    if (!successfulReply(deviceInfo, 0x81, 11) ||
        !successfulReply(systemInfo, 0x82, 16) ||
        !successfulReply(versionInfo, 0x84, 6))
        return {};

    uint32_t mask = 0;
    for (unsigned i = 0; i < 4; ++i)
        mask += signedByte(deviceInfo[6 + i]) << (8 * i);
    const uint32_t factor = littleEndian(systemInfo, 7);
    uint32_t key = littleEndian(systemInfo, 11);
    // Upstream model-specific exception; not independently verified on QZ.
    if (key == 370357u && factor == 39915u)
        key = 374677u;

    Bytes result(36);
    for (unsigned i = 0; i < 32; ++i) {
        const uint32_t b = i + 1;
        uint32_t mixed;
        if (((mask >> i) & 1u) == 0)
            mixed = (i + factor) * b;
        else if (i < 16)
            mixed = arithmeticShift((key << 16) | arithmeticShift(key, 16), i);
        else
            mixed = arithmeticShift(key, i);
        result[i] = static_cast<uint8_t>(b ^ mixed);
    }
    const uint32_t secret = signedByte(versionInfo[4]) * 8u;
    for (unsigned i = 0; i < 4; ++i)
        result[32 + i] = static_cast<uint8_t>(secret >> (8 * i));
    return result;
}
} // namespace FitProProtocol
#endif
