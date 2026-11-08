#ifndef KETTLERC12PROTOCOL_H
#define KETTLERC12PROTOCOL_H

#include <array>
#include <cstdint>
#include <cstddef>

namespace KettlerC12Protocol {

// ATT handle observed on the ERGO C12 in the Kinomap HCI capture.
// This is a firmware-specific fallback, not a portable characteristic UUID.
constexpr uint16_t targetPowerHandle = 0x0042;
constexpr int maxTargetPowerWatts = 250;
constexpr int minResistanceLevel = 1;
constexpr int maxResistanceLevel = 20;

// Observed on the ERGO C12: Kinomap reads 638a1104 (ATT 0x30),
// receives 3e 7c, and sends exactly 16 bytes to 638a1105 (ATT 0x32).
// A single capture does not establish a general authentication algorithm.
// Never replay this one response for a different challenge.
inline bool matchesCapturedChallenge(const uint8_t *value, std::size_t length) {
    return length == 2 && value[0] == 0x3e && value[1] == 0x7c;
}

inline std::array<uint8_t, 16> capturedHandshakeResponse() {
    return {{0x4e, 0x0e, 0x07, 0x9d, 0xe0, 0xe7, 0xba, 0x94,
             0x74, 0xaa, 0xab, 0x50, 0x3c, 0x5f, 0x4c, 0xc3}};
}

inline uint8_t encodeResistanceLevel(int level) {
    return static_cast<uint8_t>(level < minResistanceLevel ? minResistanceLevel :
                                level > maxResistanceLevel ? maxResistanceLevel : level);
}

inline std::array<uint8_t, 2> encodeTargetPower(int watts) {
    if (watts < 0)
        watts = 0;
    else if (watts > maxTargetPowerWatts)
        watts = maxTargetPowerWatts;

    return {{static_cast<uint8_t>(watts & 0xff),
             static_cast<uint8_t>((watts >> 8) & 0xff)}};
}

} // namespace KettlerC12Protocol

#endif // KETTLERC12PROTOCOL_H
