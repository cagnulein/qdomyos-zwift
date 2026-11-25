// SPDX-License-Identifier: GPL-3.0-or-later
// Unlock algorithm adapted from OpenFit (BaconWappedBitcoin/openfit),
// android/protocol/src/main/kotlin/com/ofit/protocol/UnlockCalculator.kt,
// GPL-3.0-or-later. See docs/fitpro-protocol.md for attribution and evidence.
#ifndef FITPROPROTOCOL_H
#define FITPROPROTOCOL_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <map>
#include <cmath>

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
using Fields = std::map<unsigned, uint32_t>;
using FieldIds = std::vector<unsigned>;
inline unsigned fieldSize(unsigned id) {
    switch (id) {
    case 12: case 36: case 95: case 96: case 100: case 108: return 1;
    case 0: case 1: case 3: case 15: case 16: case 17: case 27: case 28:
    case 30: case 31: case 34: case 35: case 46: case 52: case 54: case 71: return 2;
    case 4: case 10: case 20: case 21: case 69: case 70: case 75: case 76: return 4;
    default: return 0;
    }
}
inline FieldIds telemetryFields() { return {0,1,3,4,10,12,15,16,20,21,46,52,54,71,75,76,96}; }
inline FieldIds limitFields() { return {12,27,28,30,31,34,35,36,46,69,70,71,100}; }
inline Bytes bitmap(const FieldIds &ids) {
    if (ids.empty()) return {0};
    if (ids.back() > 115) return {};
    Bytes result(2 + ids.back()/8, 0);
    result[0] = static_cast<uint8_t>(result.size()-1);
    for (unsigned id : ids) {
        if (id > ids.back() || !fieldSize(id)) return {};
        result[1+id/8] |= static_cast<uint8_t>(1u << (id%8));
    }
    return result;
}
inline Bytes readFields(const FieldIds &ids) {
    const Bytes mask = bitmap(ids);
    if (mask.empty()) return {};
    Bytes result{0};
    result.insert(result.end(), mask.begin(), mask.end());
    return result;
}
inline Bytes writeFields(const Fields &fields) {
    if (fields.empty()) return {};
    FieldIds ids;
    for (const auto &f : fields) ids.push_back(f.first);
    Bytes result = bitmap(ids);
    if (result.empty()) return {};
    for (const auto &f : fields) {
        const unsigned size = fieldSize(f.first);
        if (!size || (size < 4 && f.second >= (uint32_t(1) << (size*8)))) return {};
        for (unsigned i=0; i<size; ++i) result.push_back(static_cast<uint8_t>(f.second >> (8*i)));
    }
    result.push_back(0);
    return result;
}
// No partial decoding: a different payload layout must never become a motor command.
inline bool decodeFields(const Bytes &frame, const FieldIds &ids, Fields &fields) {
    fields.clear();
    if (!successfulReply(frame, 2, 5)) return false;
    Fields result;
    std::size_t offset=4;
    for (unsigned id : ids) {
        const unsigned size=fieldSize(id);
        if (!size || offset+size > frame.size()-1 || result.count(id)) return false;
        uint32_t value=0;
        for (unsigned i=0;i<size;++i) value |= uint32_t(frame[offset++]) << (8*i);
        result[id]=value;
    }
    if (offset != frame.size()-1) return false;
    fields.swap(result); return true;
}
inline double signedHundredths(uint32_t v) { return (v<0x8000u ? double(v) : double(v)-65536.0)/100.0; }
inline uint32_t wireHundredths(double v) { return static_cast<uint16_t>(static_cast<int>(std::round(v*100.0))); }
inline bool within(double v, double low, double high) {
    return std::isfinite(v) && std::isfinite(low) && std::isfinite(high) &&
        low<=high && v>=low && v<=high && std::round(v*100.0)/100.0>=low && std::round(v*100.0)/100.0<=high;
}
class ReplyAssembler {
    Chunks chunks;
public:
    void reset() { chunks.clear(); }
    Bytes append(const Bytes &chunk) {
        if (chunk.empty()) return {};
        if (chunk[0]==0xfe) {
            reset();
            if (chunk.size()>=4 && chunk.size()<=20 && chunk[1]==2 && chunk[3]>=2 && chunk[3]<=16)
                chunks.push_back(chunk);
            return {};
        }
        if (chunks.empty()) return {};
        chunks.push_back(chunk);
        if (chunk[0]==0xff || chunks.size()>=chunks.front()[3]) {
            Bytes frame=decodeReply(chunks);
            reset(); return frame;
        }
        return {};
    }
};
} // namespace FitProProtocol
#endif
