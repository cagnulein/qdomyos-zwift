#ifndef STABLEFTMSPOWERMATCH_H
#define STABLEFTMSPOWERMATCH_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <vector>

// Conservative power matching for an FTMS trainer with an independent pedal power sensor.
// The learned offset is trainer-reported watts minus pedal watts; it is independent of a
// calculated resistance level (which changes whenever either power measurement changes).
// No persistent data are used. Target transitions and invalid sensors reset the estimate.
class StableFtmsPowerMatch {
public:
    static constexpr int MAX_ABSOLUTE_SENSOR_DELTA = 70;
    static constexpr int MAX_CORRECTION_WATTS = 50;
    static constexpr int MAX_UP_WATTS_PER_SECOND = 3;
    static constexpr int MAX_DOWN_WATTS_PER_SECOND = 8;
    static constexpr int OUTLIER_HOLD_MILLISECONDS = 4000;
    static constexpr int STABLE_ESTIMATE_TIMEOUT_MILLISECONDS = 12000;

    // When the virtual FTMS bike already drives changePower() through its target-power
    // callback, forwarding that same 0x05 packet directly to the trainer would
    // overwrite the rate-limited, pedal-matched command. Only suppress it for
    // external-sensor ERG when the trainer lacks native resistance telemetry.
    static bool shouldBlockDirectFtmsPower(bool externalPowerSensor, bool nativeResistance,
                                           bool nativeErg, bool autoResistanceEnabled) {
        return externalPowerSensor && !nativeResistance && nativeErg && autoResistanceEnabled;
    }

    void reset() {
        samples.clear();
        target = 0;
        command = 0;
        lastSampleMs = -1;
        lastCommandMs = -1;
        lastStableMs = -1;
        stableDelta = 0;
        lastStatus = "reset";
    }

    int adjustedPower(int requestedWatts, double trainerWatts, double pedalWatts,
                      double cadence, int64_t nowMs) {
        if (requestedWatts <= 0) {
            reset();
            return requestedWatts;
        }

        if (requestedWatts != target || (lastCommandMs >= 0 && nowMs < lastCommandMs)) {
            reset();
            target = requestedWatts;
            command = requestedWatts;
            lastCommandMs = nowMs;
            lastStatus = "target-change";
            return command; // Never carry an offset across structured-workout intervals.
        }

        if (lastCommandMs >= 0 && nowMs - lastCommandMs < 900)
            return command;

        // A missing/stale sensor is a safety event, unlike one outlying pair.
        // Release any learned offset immediately, then ramp down at the normal
        // limited rate; after BLE reconnect we must learn from fresh data.
        const bool sensorAvailable =
            std::isfinite(trainerWatts) && std::isfinite(pedalWatts) &&
            std::isfinite(cadence) && trainerWatts > 0 && pedalWatts > 0 &&
            cadence >= 60 && cadence <= 130;
        const bool safeDelta = sensorAvailable &&
                               std::abs(trainerWatts - pedalWatts) <= MAX_ABSOLUTE_SENSOR_DELTA;
        bool newStableEstimate = false;

        if (!sensorAvailable) {
            samples.clear();
            lastSampleMs = -1;
            lastStableMs = -1;
            stableDelta = 0;
            lastStatus = "sensor-missing";
        } else if (!safeDelta) {
            // A single trainer spike must not erase the previously validated
            // offset. Do not sample it or increase the trainer command either.
            lastStatus = "outlier-hold";
        } else if (lastSampleMs < 0 || nowMs - lastSampleMs >= 900) {
            if (lastSampleMs >= 0 && nowMs - lastSampleMs > 15000)
                samples.clear();
            lastSampleMs = nowMs;
            samples.push_back({nowMs, trainerWatts, pedalWatts, cadence});
            while (samples.size() > 9)
                samples.pop_front();
            if (stableWindow()) {
                std::vector<double> deltas;
                for (const auto &sample : samples)
                    deltas.push_back(sample.trainer - sample.pedal);
                std::sort(deltas.begin(), deltas.end());
                const double median = deltas[4];
                const int maxOffset = std::min(MAX_CORRECTION_WATTS,
                                                static_cast<int>(requestedWatts * 0.30));
                stableDelta = std::max(-maxOffset, std::min(maxOffset,
                                              static_cast<int>(std::lround(median))));
                lastStableMs = nowMs;
                newStableEstimate = true;
                lastStatus = "stable";
            } else {
                lastStatus = "window-unstable";
            }
        }

        // A higher pedal reading always takes precedence over a held estimate.
        if (sensorAvailable && pedalWatts > requestedWatts + 15) {
            stableDelta = std::min(stableDelta, 0);
            lastStatus = "pedal-over-target";
        }

        // When recent samples are noisy, freeze the command rather than
        // oscillating between an increased command and the base target.
        // Prolonged noise (or an actual sensor loss) returns safely to base.
        const bool estimateFresh = sensorAvailable && lastStableMs >= 0 &&
            nowMs - lastStableMs <= STABLE_ESTIMATE_TIMEOUT_MILLISECONDS;
        const bool holdCommand = estimateFresh && !newStableEstimate &&
                                 nowMs - lastStableMs <= OUTLIER_HOLD_MILLISECONDS &&
                                 !(pedalWatts > requestedWatts + 15);
        int desired = requestedWatts;
        if (estimateFresh && newStableEstimate)
            desired += stableDelta;
        else if (holdCommand)
            desired = command;
        else if (!estimateFresh && sensorAvailable) {
            stableDelta = 0;
            lastStatus = "estimate-expired";
        } else if (estimateFresh) {
            // No fresh stable estimate: do not increase any further.
            // Hold the last command briefly, then ramp back to base.
            lastStatus = "unstable-fallback";
        }

        const int64_t elapsed = std::max<int64_t>(0, nowMs - lastCommandMs);
        const int upStep = std::max(1, static_cast<int>(elapsed * MAX_UP_WATTS_PER_SECOND / 1000));
        const int downStep = std::max(1, static_cast<int>(elapsed * MAX_DOWN_WATTS_PER_SECOND / 1000));
        command += std::max(-downStep, std::min(upStep, desired - command));
        lastCommandMs = nowMs;
        return command;
    }

    int currentCorrection() const { return command - target; }
    bool hasStableEstimate() const { return lastStableMs >= 0; }
    const char *status() const { return lastStatus; }

private:
    struct Sample {
        int64_t when;
        double trainer;
        double pedal;
        double cadence;
    };

    bool stableWindow() const {
        // Nine recent pairs and a trimmed median tolerate up to two outliers
        // without accepting a generally unstable relationship.
        if (samples.size() != 9 ||
            samples.back().when - samples.front().when < 6500 ||
            samples.back().when - samples.front().when > 15000)
            return false;

        std::vector<double> deltas;
        std::vector<double> cadences;
        for (const auto &s : samples) {
            deltas.push_back(s.trainer - s.pedal);
            cadences.push_back(s.cadence);
        }
        std::sort(deltas.begin(), deltas.end());
        std::sort(cadences.begin(), cadences.end());
        return deltas[6] - deltas[2] <= 14 &&
               cadences[7] - cadences[1] <= 10;
    }

    std::deque<Sample> samples;
    int target = 0;
    int command = 0;
    int64_t lastSampleMs = -1;
    int64_t lastCommandMs = -1;
    int64_t lastStableMs = -1;
    int stableDelta = 0;
    const char *lastStatus = "reset";
};

#endif // STABLEFTMSPOWERMATCH_H
