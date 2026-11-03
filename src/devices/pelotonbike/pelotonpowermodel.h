#ifndef PELOTONPOWERMODEL_H
#define PELOTONPOWERMODEL_H

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

// Bike+ power-to-resistance seed model from doudar/grupetto (PowerModel.kt).
// Fitted there from 3,662 one-second Bike+ measurements. Learn a correction
// from this bike's own steady telemetry instead of assuming identical calibration.
class PelotonPowerModel {
  public:
    PelotonPowerModel() { m_wattsAt90.fill(std::numeric_limits<float>::quiet_NaN()); }

    static float seedWattsAt90(int level) {
        const float r = static_cast<float>(std::max(0, std::min(level, 100)));
        return std::exp(1.491016f + 0.108267f * r - 0.000584f * r * r);
    }

    float wattsAt90(int level) const {
        level = std::max(0, std::min(level, 100));
        if (std::isfinite(m_wattsAt90[level]))
            return m_wattsAt90[level];

        // Scale the seed curve by the nearest measured level when available.
        for (int distance = 1; distance <= 5; ++distance) {
            for (const int nearby : {level - distance, level + distance}) {
                if (nearby >= 0 && nearby <= 100 && std::isfinite(m_wattsAt90[nearby])) {
                    const float scale = m_wattsAt90[nearby] / seedWattsAt90(nearby);
                    return seedWattsAt90(level) * std::max(2.0f / 3.0f, std::min(scale, 1.5f));
                }
            }
        }
        return seedWattsAt90(level);
    }

    float predictedWatts(int level, float cadence) const {
        if (cadence <= 0.0f)
            return 0.0f;
        return wattsAt90(level) * std::pow(cadence / 90.0f, 1.7f);
    }

    void learn(int level, float power, float cadence) {
        if (level < 0 || level > 100 || cadence < 50.0f || power <= 5.0f ||
            !std::isfinite(power) || !std::isfinite(cadence))
            return;

        const float observed = power / std::pow(cadence / 90.0f, 1.7f);
        const float seed = seedWattsAt90(level);
        if (observed < seed / 1.5f || observed > seed * 1.5f)
            return;

        float &stored = m_wattsAt90[level];
        stored = std::isfinite(stored) ? stored + 0.2f * (observed - stored) : observed;
    }

    // Find the closest whole resistance level for the target watts at this RPM.
    // The model was fitted at levels 18-63, so do not feed-forward beyond 75.
    int resistanceFor(float power, float cadence) const {
        if (!std::isfinite(power) || !std::isfinite(cadence) || power <= 0.0f || cadence < 50.0f)
            return -1;

        int bestLevel = 0;
        float bestError = std::numeric_limits<float>::max();
        for (int level = 0; level <= 75; ++level) {
            const float error = std::fabs(predictedWatts(level, cadence) - power);
            if (error < bestError) {
                bestError = error;
                bestLevel = level;
            }
        }
        return bestLevel;
    }

  private:
    std::array<float, 101> m_wattsAt90;
};

#endif // PELOTONPOWERMODEL_H
