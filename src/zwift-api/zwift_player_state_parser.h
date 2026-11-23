#ifndef ZWIFT_PLAYER_STATE_PARSER_H
#define ZWIFT_PLAYER_STATE_PARSER_H

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

// Decode only the fields used by QZ's auto-incline (PlayerState.distance = 3,
// PlayerState.altitude = 26). This avoids a platform-specific Protobuf runtime
// dependency on Raspberry Pi and other non-MSVC desktop builds.
namespace zwift_player_state {
namespace detail {
inline bool readVarint(const std::uint8_t *&cursor, const std::uint8_t *end, std::uint64_t &value) {
    value = 0;
    for (unsigned i = 0; i < 10 && cursor < end; ++i) {
        const std::uint8_t byte = *cursor++;
        if (i == 9 && byte > 1) // A 64-bit varint has at most one bit in byte ten.
            return false;
        value |= static_cast<std::uint64_t>(byte & 0x7f) << (7 * i);
        if (!(byte & 0x80))
            return true;
    }
    return false;
}
} // namespace detail

inline bool parse(const char *data, std::size_t size, float &distance, float &altitude) {
    if (!data || !size)
        return false;

    const auto *cursor = reinterpret_cast<const std::uint8_t *>(data);
    const auto *end = cursor + size;
    float parsedDistance = 0;
    float parsedAltitude = 0;
    bool foundDistance = false;

    while (cursor < end) {
        std::uint64_t tag;
        if (!detail::readVarint(cursor, end, tag) || tag < 8)
            return false;

        const std::uint64_t field = tag >> 3;
        const unsigned wireType = static_cast<unsigned>(tag & 7);
        if ((field == 3 && wireType != 0) || (field == 26 && wireType != 5))
            return false;

        switch (wireType) {
        case 0: {
            std::uint64_t value;
            if (!detail::readVarint(cursor, end, value))
                return false;
            if (field == 3) {
                if (value > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max()))
                    return false;
                parsedDistance = static_cast<float>(value);
                foundDistance = true;
            }
            break;
        }
        case 1: // 64-bit value (unknown field).
            if (end - cursor < 8)
                return false;
            cursor += 8;
            break;
        case 2: { // Length-delimited value (unknown field).
            std::uint64_t length;
            if (!detail::readVarint(cursor, end, length) ||
                length > static_cast<std::uint64_t>(end - cursor))
                return false;
            cursor += static_cast<std::size_t>(length);
            break;
        }
        case 5: { // 32-bit little-endian value.
            if (end - cursor < 4)
                return false;
            if (field == 26) {
                const std::uint32_t bits = static_cast<std::uint32_t>(cursor[0]) |
                                           (static_cast<std::uint32_t>(cursor[1]) << 8) |
                                           (static_cast<std::uint32_t>(cursor[2]) << 16) |
                                           (static_cast<std::uint32_t>(cursor[3]) << 24);
                std::memcpy(&parsedAltitude, &bits, sizeof(parsedAltitude));
                if (!std::isfinite(parsedAltitude))
                    return false;
            }
            cursor += 4;
            break;
        }
        default: // Groups are not part of the PlayerState schema.
            return false;
        }
    }

    // A nonempty unrelated/partial protobuf message must not reset the
    // previous incline sample to (0, 0).
    if (!foundDistance)
        return false;
    distance = parsedDistance;
    altitude = parsedAltitude;
    return true;
}
} // namespace zwift_player_state

#endif // ZWIFT_PLAYER_STATE_PARSER_H
