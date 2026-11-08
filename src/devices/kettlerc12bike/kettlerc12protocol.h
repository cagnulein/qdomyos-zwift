#ifndef KETTLERC12PROTOCOL_H
#define KETTLERC12PROTOCOL_H

#include <array>
#include <cstdint>

namespace KettlerC12Protocol {

// ATT handle observed on the ERGO C12 in the Kinomap HCI capture.
// This is a firmware-specific fallback, not a portable characteristic UUID.
constexpr uint16_t targetPowerHandle = 0x0042;
constexpr int maxTargetPowerWatts = 250;

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
