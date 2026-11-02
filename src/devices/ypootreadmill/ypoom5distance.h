#ifndef YPOOM5DISTANCE_H
#define YPOOM5DISTANCE_H

#include <cstdint>

// One distance clock driven by M5 telemetry, independent of the UI update timer.
class YpooM5Distance {
  public:
    void reset() { *this = YpooM5Distance(); }

    double updateCounter(int64_t now, uint32_t meters, bool recording) {
        if (lastCounter >= 0 && now < lastCounterChanged)
            return 0;
        double increase = 0;
        if (lastCounter >= 0 && meters >= lastCounter) {
            increase = meters - lastCounter;
            // Use the same plausibility limit as Horizon's direct-distance mode.
            const double maximumIncrease = 100.0 + 40.0 * (now - lastCounterChanged) / 3600.0;
            if (increase > maximumIncrease)
                return 0; // Keep the last valid baseline after a corrupt positive spike.
        }
        if (meters != lastCounter)
            lastCounterChanged = now;
        // First samples and counter resets establish a baseline without adding distance.
        lastCounter = meters;
        return recording ? increase / 1000.0 : 0;
    }

    double update(int64_t now, double speed, bool recording) {
        const int64_t elapsed = now - lastSample;
        const double distance = recording && lastSample >= 0 && elapsed > 0 && elapsed <= 2000
                                    ? lastSpeed * elapsed / 3600000.0
                                    : 0;
        // Paused samples still advance the baseline: resuming must not count paused time.
        lastSample = now;
        lastSpeed = speed;
        return distance;
    }

  private:
    int64_t lastSample = -1;
    double lastSpeed = 0;
    int64_t lastCounter = -1;
    int64_t lastCounterChanged = -1;
};

#endif // YPOOM5DISTANCE_H
