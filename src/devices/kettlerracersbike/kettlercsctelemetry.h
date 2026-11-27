#ifndef KETTLERCSCTELEMETRY_H
#define KETTLERCSCTELEMETRY_H

#include <cstdint>

namespace kettler {
inline uint16_t readLe16(const char *data)
{
    return static_cast<uint16_t>(static_cast<uint8_t>(data[0]) |
                                 (static_cast<uint16_t>(static_cast<uint8_t>(data[1])) << 8));
}

inline uint32_t readLe32(const char *data)
{
    return static_cast<uint32_t>(readLe16(data)) |
           (static_cast<uint32_t>(readLe16(data + 2)) << 16);
}

inline double wheelSpeedKmh(uint32_t revolutions, uint32_t previousRevolutions,
                            uint16_t eventTime, uint16_t previousEventTime)
{
    const uint32_t revolutionDelta = revolutions - previousRevolutions;
    const uint16_t timeDelta = static_cast<uint16_t>(eventTime - previousEventTime);
    // CSC time is in 1/1024 seconds; wheel circumference is 2.1 metres.
    return timeDelta ? revolutionDelta * 2.1 * 1024.0 * 3.6 / timeDelta : 0.0;
}
}

#endif
