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

    void reset() {
        samples.clear();
        target = 0;
        command = 0;
        lastSampleMs = -1;
        lastCommandMs = -1;
        lastStableMs = -1;
        stableDelta = 0;
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
            return command; // Never carry an offset across structured-workout intervals.
        }

        if (lastCommandMs >= 0 && nowMs - lastCommandMs < 900)
            return command;

        const bool valid = std::isfinite(trainerWatts) && std::isfinite(pedalWatts) &&
                           std::isfinite(cadence) && trainerWatts > 0 && pedalWatts > 0 &&
                           cadence >= 60 && cadence <= 130 &&
                           std::abs(trainerWatts - pedalWatts) <= MAX_ABSOLUTE_SENSOR_DELTA &&
                           std::abs(trainerWatts - command) <= 20;

        if (!valid) {
            samples.clear();
            stableDelta = 0;
            lastStableMs = -1;
        } else if (lastSampleMs < 0 || nowMs - lastSampleMs >= 900) {
            lastSampleMs = nowMs;
            samples.push_back({nowMs, trainerWatts, pedalWatts, cadence});
            while (samples.size() > 6)
                samples.pop_front();
            if (stableWindow()) {
                std::vector<double> deltas;
                for (const auto &sample : samples)
                    deltas.push_back(sample.trainer - sample.pedal);
                std::sort(deltas.begin(), deltas.end());
                const double median = (deltas[2] + deltas[3]) / 2.0;
                const int maxOffset = std::min(MAX_CORRECTION_WATTS,
                                                static_cast<int>(requestedWatts * 0.30));
                stableDelta = std::max(-maxOffset, std::min(maxOffset,
                                              static_cast<int>(std::lround(median))));
                lastStableMs = nowMs;
            }
        }

        // Never keep adding power when the pedals already exceed the requested output.
        if (pedalWatts > requestedWatts + 15)
            stableDelta = std::min(stableDelta, 0);

        // After a transient, hold the last stable estimate briefly; then ramp back to
        // the uncorrected target. A lost sensor never leaves a latched high command.
        const bool fresh = valid && lastStableMs >= 0 && nowMs - lastStableMs <= 5000;
        const int desired = requestedWatts + (fresh ? stableDelta : 0);
        const int64_t elapsed = std::max<int64_t>(0, nowMs - lastCommandMs);
        const int upStep = std::max(1, static_cast<int>(elapsed * MAX_UP_WATTS_PER_SECOND / 1000));
        const int downStep = std::max(1, static_cast<int>(elapsed * MAX_DOWN_WATTS_PER_SECOND / 1000));
        command += std::max(-downStep, std::min(upStep, desired - command));
        lastCommandMs = nowMs;
        return command;
    }

    int currentCorrection() const { return command - target; }
    bool hasStableEstimate() const { return lastStableMs >= 0; }

private:
    struct Sample {
        int64_t when;
        double trainer;
        double pedal;
        double cadence;
    };

    bool stableWindow() const {
        if (samples.size() != 6 || samples.back().when - samples.front().when < 4000)
            return false;

        double minDelta = 1e9, maxDelta = -1e9;
        double minTrainer = 1e9, maxTrainer = -1e9;
        double minPedal = 1e9, maxPedal = -1e9;
        double minCadence = 1e9, maxCadence = -1e9;
        for (const auto &s : samples) {
            const double delta = s.trainer - s.pedal;
            minDelta = std::min(minDelta, delta);
            maxDelta = std::max(maxDelta, delta);
            minTrainer = std::min(minTrainer, s.trainer);
            maxTrainer = std::max(maxTrainer, s.trainer);
            minPedal = std::min(minPedal, s.pedal);
            maxPedal = std::max(maxPedal, s.pedal);
            minCadence = std::min(minCadence, s.cadence);
            maxCadence = std::max(maxCadence, s.cadence);
        }
        return maxDelta - minDelta <= 12 && maxTrainer - minTrainer <= 20 &&
               maxPedal - minPedal <= 20 && maxCadence - minCadence <= 10;
    }

    std::deque<Sample> samples;
    int target = 0;
    int command = 0;
    int64_t lastSampleMs = -1;
    int64_t lastCommandMs = -1;
    int64_t lastStableMs = -1;
    int stableDelta = 0;
};

#endif // STABLEFTMSPOWERMATCH_H
