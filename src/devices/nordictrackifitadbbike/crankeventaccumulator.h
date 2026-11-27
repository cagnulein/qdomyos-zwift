#ifndef CRANKEVENTACCUMULATOR_H
#define CRANKEVENTACCUMULATOR_H

#include <cmath>
#include <cstdint>

// Integrates sampled RPM against a monotonic millisecond clock. BLE crank
// event timestamps are modulo 65536 in units of 1/1024 second.
class CrankEventAccumulator {
  public:
    uint64_t update(int64_t nowMs, double rpm, uint16_t &eventTime) {
        const int64_t elapsed = nowMs - lastMs;
        const double previousRpm = lastRpm;
        lastMs = nowMs;
        lastRpm = std::isfinite(rpm) && rpm > 0 ? rpm : 0;
        // Do not fabricate revolutions before the first sample, at a stop,
        // or across a long telemetry interruption.
        if (elapsed <= 0 || elapsed > 2000 || previousRpm <= 0 || lastRpm <= 0) {
            if (elapsed < 0 || elapsed > 2000 || previousRpm <= 0 || lastRpm <= 0)
                phase = 0;
            return 0;
        }
        phase += previousRpm * elapsed;
        const auto revolutions = static_cast<uint64_t>(phase / 60000.0);
        if (revolutions) {
            phase -= revolutions * 60000.0;
            const double eventMs = nowMs - phase / previousRpm;
            eventTime = static_cast<uint16_t>(static_cast<uint64_t>(eventMs * 1.024));
        }
        return revolutions;
    }

  private:
    int64_t lastMs = 0;
    double lastRpm = 0;
    double phase = 0; // RPM * ms, 60000 per revolution
};

#endif
