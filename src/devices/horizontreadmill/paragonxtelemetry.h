#pragma once

#include <cstddef>
#include <cstdint>

// The Paragon X reports speed three times in its 0x17 status packet.
// Some iOS connections deliver only the first three 20-byte BLE notifications
// (60 bytes) before the next status packet starts. The missing bytes are at
// the end; speed and inclination are already present in the received prefix.
namespace ParagonXTelemetry {
struct Metrics {
    double speedKmh;
    double inclinePercent;
};

inline bool decode(const uint8_t *packet, std::size_t size, bool allowIncomplete, Metrics &metrics) {
    if (!packet || size < 31 || packet[0] != 0x55 || packet[1] != 0xaa || packet[4] != 0x02 ||
        packet[5] != 0x17 || packet[6] != 0x40 || packet[7] != 0x00) {
        return false;
    }

    const std::size_t expected = 10 + (static_cast<std::size_t>(packet[7]) << 8) + packet[6];
    if (size < expected) {
        // Never decode incomplete data until the *next* packet header has
        // proven that the previous packet will not receive more fragments.
        if (!allowIncomplete || size < 60)
            return false;
        // An incomplete frame lacks its trailing checksum/footer: check the
        // three redundant speed copies to avoid promoting corrupted data.
        if (packet[24] != packet[26] || packet[25] != packet[27] ||
            packet[24] != packet[28] || packet[25] != packet[29])
            return false;
    }

    const uint16_t rawSpeed = static_cast<uint16_t>(packet[24]) |
                              (static_cast<uint16_t>(packet[25]) << 8);
    const double speed = (static_cast<double>(rawSpeed) / 100.0) * 1.60934;
    const double incline = static_cast<double>(packet[30]) / 10.0;
    if (speed > 40.0 || incline > 30.0)
        return false;

    metrics.speedKmh = speed;
    metrics.inclinePercent = incline;
    return true;
}

// Only a subsequent frame header makes a truncated frame eligible for the
// conservative fallback. A BLE gap or a 20-byte fragment alone is not enough.
inline bool decodePreviousAtBoundary(const uint8_t *previous, std::size_t previousSize,
                                     int remainingBytes, const uint8_t *next,
                                     std::size_t nextSize, Metrics &metrics) {
    if (remainingBytes <= 0 || !next || nextSize <= 7 || next[0] != 0x55 || next[1] != 0xaa)
        return false;
    return decode(previous, previousSize, true, metrics);
}
} // namespace ParagonXTelemetry
