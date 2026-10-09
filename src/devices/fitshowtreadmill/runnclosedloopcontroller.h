#ifndef FITSHOW_RUNN_CLOSED_LOOP_CONTROLLER_H
#define FITSHOW_RUNN_CLOSED_LOOP_CONTROLLER_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <utility>
#include <vector>

// No Qt dependency: the control policy can be tested with synthetic sensor traces.
// Call from the FitShow driver's event loop; all speeds are km/h and times are ms.
class FitShowRunnClosedLoopController {
public:
    struct Result {
        bool changed = false;
        bool cancelled = false;
        double command = 0.0;
        double filtered = 0.0;
        const char *reason = "";
    };

    void start(double target, double initialCommand, std::int64_t nowMs) {
        reset();
        if (!std::isfinite(target) || !std::isfinite(initialCommand) ||
            target <= 0.0 || initialCommand <= 0.0) {
            return;
        }
        m_target = target;
        m_command = initialCommand;
        m_lastCommandMs = nowMs;
        m_active = true;
    }

    void reset() {
        m_active = false;
        m_everRunning = false;
        m_reachedCommand = false;
        m_target = 0.0;
        m_command = 0.0;
        m_lastCommandMs = 0;
        m_reachedAtMs = 0;
        m_lastCorrectionMs = 0;
        m_lastSampleMs = -1;
        m_mismatchSinceMs = -1;
        m_samples.clear();
    }

    bool active() const { return m_active; }
    double target() const { return m_target; }
    double command() const { return m_command; }

    void sample(double speedKmh, std::int64_t timestampMs) {
        if (!m_active || !std::isfinite(speedKmh) || speedKmh < 5.0 ||
            timestampMs <= m_lastSampleMs || timestampMs < m_lastCommandMs) {
            return;
        }
        m_lastSampleMs = timestampMs;
        m_samples.emplace_back(timestampMs, speedKmh);
        trimSamples(timestampMs);
    }

    // machineSpeed is the speed reported by the physical treadmill, not Runn.
    // maxRelativeCorrection is the existing power sensor correction threshold (fraction).
    Result update(std::int64_t nowMs, bool running, double machineSpeed,
                  std::int64_t machineSampleAgeMs, double minSpeed, double maxSpeed,
                  double maxRelativeCorrection) {
        Result result;
        if (!m_active) return result;
        if (!running) {
            if (m_everRunning) {
                reset();
                result.cancelled = true;
                result.reason = "treadmill stopped";
            }
            return result;
        }
        m_everRunning = true;

        if (machineSampleAgeMs > 3000 || machineSampleAgeMs < 0 ||
            !std::isfinite(machineSpeed) || machineSpeed < 0.0) {
            return result;
        }

        const double machineError = std::fabs(machineSpeed - m_command);
        if (!m_reachedCommand) {
            if (machineError > 0.25) return result;
            m_reachedCommand = true;
            m_reachedAtMs = nowMs;
            m_mismatchSinceMs = -1;
        } else if (machineError > 0.4) {
            // A console speed change is not a new QZ target. Never fight the user.
            if (m_mismatchSinceMs < 0) m_mismatchSinceMs = nowMs;
            if (nowMs - m_mismatchSinceMs >= 2500) {
                reset();
                result.cancelled = true;
                result.reason = "physical speed changed outside QZ";
            }
            return result;
        } else {
            m_mismatchSinceMs = -1;
        }

        if (nowMs - m_lastCommandMs < 8000 || nowMs - m_reachedAtMs < 4000 ||
            (m_lastCorrectionMs != 0 && nowMs - m_lastCorrectionMs < 5000)) {
            return result;
        }

        trimSamples(nowMs);
        if (m_samples.size() < 5 || m_lastSampleMs < nowMs - 2500 ||
            m_samples.back().first - m_samples.front().first < 4000) {
            return result;
        }

        std::vector<double> readings;
        readings.reserve(m_samples.size());
        for (const auto &sample : m_samples) readings.push_back(sample.second);
        std::sort(readings.begin(), readings.end());
        if (readings.back() - readings.front() > 2.0) return result;

        // Trim the smallest and largest samples to damp Runn sticker/stride jitter.
        const std::size_t trim = readings.size() >= 7 ? 1 : 0;
        double sum = 0.0;
        for (std::size_t i = trim; i < readings.size() - trim; ++i)
            sum += readings[i];
        const double filtered = sum / static_cast<double>(readings.size() - 2 * trim);
        result.filtered = filtered;

        const double error = m_target - filtered;
        if (std::fabs(error) <= 0.15) return result;
        if (!std::isfinite(maxRelativeCorrection) || maxRelativeCorrection <= 0.0)
            return result;

        // Reuse the configured safety threshold and impose a hard motor limit.
        const double limit = std::min(1.5, m_target * maxRelativeCorrection);
        if (std::fabs(error) > limit || !std::isfinite(maxSpeed) ||
            !std::isfinite(minSpeed) || maxSpeed <= minSpeed) {
            return result;
        }
        const double delta = std::max(-0.2, std::min(0.2, error * 0.35));
        double next = std::round((m_command + delta) * 10.0) / 10.0;
        const double lower = std::max(minSpeed, m_target - limit);
        const double upper = std::min(maxSpeed, m_target + limit);
        if (lower > upper) return result;
        next = std::max(lower, std::min(upper, next));
        if (std::fabs(next - m_command) < 0.09) return result;

        m_command = next;
        m_lastCommandMs = nowMs;
        m_lastCorrectionMs = nowMs;
        m_reachedCommand = false;
        m_reachedAtMs = 0;
        m_mismatchSinceMs = -1;
        m_lastSampleMs = -1;
        m_samples.clear();
        result.changed = true;
        result.command = next;
        result.reason = "filtered Runn speed correction";
        return result;
    }

private:
    void trimSamples(std::int64_t nowMs) {
        while (!m_samples.empty() && m_samples.front().first < nowMs - 6000)
            m_samples.pop_front();
    }

    bool m_active = false;
    bool m_everRunning = false;
    bool m_reachedCommand = false;
    double m_target = 0.0;
    double m_command = 0.0;
    std::int64_t m_lastCommandMs = 0;
    std::int64_t m_reachedAtMs = 0;
    std::int64_t m_lastCorrectionMs = 0;
    std::int64_t m_lastSampleMs = -1;
    std::int64_t m_mismatchSinceMs = -1;
    std::deque<std::pair<std::int64_t, double>> m_samples;
};

#endif // FITSHOW_RUNN_CLOSED_LOOP_CONTROLLER_H
