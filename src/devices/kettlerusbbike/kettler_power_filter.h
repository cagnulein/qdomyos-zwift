#pragma once

// Suppress brief 10 W dropouts without masking sustained low power.
class KettlerPowerFilter {
public:
    double apply(double raw, double cadence, long long nowMs) {
        if (raw == 10.0 && cadence > 0 && lastPower >= 30.0) {
            if (dropoutStart < 0)
                dropoutStart = nowMs;
            if (nowMs - dropoutStart < 4000)
                return lastPower;
        }
        dropoutStart = -1;
        lastPower = raw;
        return raw;
    }
    void reset() { lastPower = 0; dropoutStart = -1; }
private:
    double lastPower = 0;
    long long dropoutStart = -1;
};
