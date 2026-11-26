#ifndef TCFTMSOVERLAY_H
#define TCFTMSOVERLAY_H

#include <cmath>

// Model-specific power sanity checks for TC-prefix FTMS bikes. Generic FTMS
// parsing, BLE connection, and Control Point writes remain in ftmsbike.
class TcFtmsOverlay {
  public:
    bool acceptPower(double power, double cadence, int resistance, double targetPower) {
        // Preserve the last accepted reading instead of replacing anomalies with zero.
        if (!std::isfinite(power) || power < 0 || power > 1000.0)
            return false;

        // Large, sudden increases without a cadence increase are suspicious
        // when resistance did not rise much, or the active ERG target is far lower.
        if (lastPositivePower > 0 && power >= lastPositivePower * 2.5 &&
            power - lastPositivePower >= 100.0 &&
            cadence <= lastPositiveCadence + 12.0 &&
            (resistance <= lastPositiveResistance + 2 ||
             (targetPower > 0 && power >= targetPower * 2.5 && power - targetPower >= 100.0))) {
            return false;
        }

        // Keep the positive baseline across a genuine pause (0 W) so the first
        // corrupt reading after resuming does not establish a new baseline.
        if (power > 0) {
            lastPositivePower = power;
            lastPositiveCadence = cadence;
            lastPositiveResistance = resistance;
        }
        return true;
    }

  private:
    double lastPositivePower = 0;
    double lastPositiveCadence = 0;
    int lastPositiveResistance = -1;
};

#endif // TCFTMSOVERLAY_H
