#ifndef ROUVYGEARING_H
#define ROUVYGEARING_H

#include <QtGlobal>

// Exact Rouvy virtual gear values (protocol ratio * 10000).
// Do not derive them from visual chainring/cog approximations.
namespace RouvyGearing {
constexpr int GearCount = 24;

inline const int *ratiosForProfile(int profile) {
    static constexpr int profiles[4][GearCount] = {
        // Mixed Terrain
        {7500, 8700, 9800, 11000, 12000, 13100, 14100, 15300,
         16700, 18300, 20400, 22000, 23900, 26200, 27500, 30600,
         32400, 34400, 36700, 39300, 42300, 45800, 50000, 55000 },
        // Flat
        {13899, 15000, 16200, 18900, 19500, 20400, 21700, 22100,
         24400, 26000, 26500, 27900, 29400, 30000, 32500, 33100,
         35300, 35500, 37900, 39000, 40800, 44200, 48200, 53000 },
        // All-Arounder
        {10600, 12500, 14500, 14600, 16700, 17100, 18400, 20000,
         20600, 22900, 23300, 25000, 25299, 26900, 28200, 29200,
         31800, 32000, 34300, 35000, 36900, 40000, 43600, 48000 },
        // Climbing
        {8300, 9400, 10700, 11900, 12500, 13400, 14300, 15400,
         15800, 17600, 17900, 20000, 20500, 22599, 23100, 25000,
         25299, 27300, 28700, 30000, 33100, 35800, 39100, 43000 }
    };
    return (profile >= 0 && profile < 4) ? profiles[profile] : nullptr;
}

inline int gearForRatio(int profile, qint64 ratio) {
    const int *values = ratiosForProfile(profile);
    if (!values)
        return 0;
    for (int i = 0; i < GearCount; ++i) {
        if (ratio == values[i])
            return i + 1;
    }
    return 0;
}

inline int ratioForGear(int profile, int gear) {
    const int *values = ratiosForProfile(profile);
    if (!values || gear < 1 || gear > GearCount)
        return 0;
    return values[gear - 1];
}
} // namespace RouvyGearing

#endif // ROUVYGEARING_H
