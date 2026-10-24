#ifndef YPOOM5DISTANCE_H
#define YPOOM5DISTANCE_H

#include <cstdint>

// One distance clock driven by M5 telemetry, independent of the UI update timer.
class YpooM5Distance {
  public:
    void reset() { *this = YpooM5Distance(); }

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
};

#endif // YPOOM5DISTANCE_H
