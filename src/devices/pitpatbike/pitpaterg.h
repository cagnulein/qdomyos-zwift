#pragma once

#include <cstdint>

namespace PitPatErg {
inline uint16_t estimatedWatts(double cadence, double resistance) {
    if (cadence <= 0.0)
        return 0;

    // The S01PRO native profile reports an unrealistic cadence-only power value
    // (roughly cadence * 4), independent of resistance. For QZ we instead use
    // resistance as a linear scale: at 100 RPM level 1 is about 40 W and level
    // 32 is about 400 W, with the 30 intermediate levels evenly interpolated.
    double level = resistance;
    if (level < 1.0)
        level = 1.0;
    else if (level > 32.0)
        level = 32.0;

    const double resistanceScale = 0.10 + (0.90 * ((level - 1.0) / 31.0));
    const double watts = cadence * 4.0 * resistanceScale;
    if (watts >= 65535.0)
        return 65535;
    return static_cast<uint16_t>(watts + 0.5);
}

// Select the closest attainable estimated power at the actual cadence.
// At equal error, prefer the lower resistance. Do not load the bike at rest.
inline uint16_t resistanceForPower(uint16_t power, double cadence) {
    if (cadence <= 0.0 || power == 0)
        return 1;

    uint16_t best = 1;
    int bestError = 65536;
    for (uint16_t level = 1; level <= 32; ++level) {
        int error = static_cast<int>(estimatedWatts(cadence, level)) - static_cast<int>(power);
        if (error < 0)
            error = -error;
        if (error < bestError) {
            best = level;
            bestError = error;
        }
    }
    return best;
}
} // namespace PitPatErg
