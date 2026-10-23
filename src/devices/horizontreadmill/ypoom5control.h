#ifndef YPOOM5CONTROL_H
#define YPOOM5CONTROL_H

#include <cmath>
#include <cstddef>
#include <cstdint>

// The M5 reports FTMS operation-failed even when a speed command is applied.
// Confirm motor commands with fresh treadmill telemetry, not response codes.
class YpooM5Control {
  public:
    enum class Action { None, Start, Stop, Speed, Incline };
    struct Command {
        Action action;
        double value;
    };

    void reset() { *this = YpooM5Control(); }
    bool running() const { return speed > 0; }

    static bool readTelemetry(const uint8_t *data, size_t size, double &reportedSpeed, double &reportedIncline) {
        if (size < 2)
            return false;
        const uint16_t flags = data[0] | (uint16_t(data[1]) << 8);
        if ((flags & 1) || !(flags & 8))
            return false; // A control confirmation needs both instantaneous speed and incline.
        const unsigned fieldSizes[] = {0, 2, 3, 4, 4, 1, 1, 5, 1, 1, 2, 2, 4};
        size_t required = 4;
        for (unsigned bit = 1; bit < 13; ++bit) {
            if (flags & (1 << bit))
                required += fieldSizes[bit];
        }
        if (size < required)
            return false;
        // Ignore RFU bits: captured M5 firmware sets bit 13 and appends extra bytes.
        reportedSpeed = (data[2] | (uint16_t(data[3]) << 8)) / 100.0;
        size_t offset = 4 + ((flags & 2) ? 2 : 0) + ((flags & 4) ? 3 : 0);
        int signedIncline = data[offset] | (uint16_t(data[offset + 1]) << 8);
        if (signedIncline >= 0x8000)
            signedIncline -= 0x10000;
        reportedIncline = signedIncline / 10.0;
        return true;
    }

    static double percentToLevel(double percent) { return std::round(percent * 17.5) / 10.0; }
    static double levelToPercent(double level) { return level * 12.0 / 21.0; }

    void requestStart() {
        if (!stopRequested && !faulted && !starting)
            startRequested = true;
    }

    void requestStop() {
        if (!stopRequested) {
            stopAttempts = 0;
            lastStopSent = -1;
            stopAcknowledged = false;
        }
        stopRequested = true;
        startRequested = false;
        speedTarget = Target();
        inclineTarget = Target();
    }

    bool requestSpeed(double value) {
        if (value == 0) {
            requestStop();
            return true;
        }
        if (!std::isfinite(value) || value < 1.0 || value > 18.8)
            return false;
        return setTarget(speedTarget, std::round(value * 10) / 10);
    }

    bool requestIncline(double value) {
        if (!std::isfinite(value) || value < -6.0 || value > 15.0)
            return false;
        return setTarget(inclineTarget, std::round(value));
    }

    void telemetry(double reportedSpeed, double reportedIncline, int64_t now) {
        if (running() && reportedSpeed == 0 && !starting) {
            speedTarget = Target();
            inclineTarget = Target();
        }
        speed = reportedSpeed;
        incline = reportedIncline;
        lastTelemetry = now;
        if (running())
            starting = false;
        if (stopRequested && stopAcknowledged && !running()) {
            stopRequested = false;
            starting = false;
        }
    }

    void stopResponse(bool success) {
        if (stopRequested && stopAttempts > 0)
            stopAcknowledged = success;
    }

    Command next(int64_t now, bool ypM1552 = false) {
        // The M5 cannot overlap procedures; leave margin above its 200 ms minimum.
        if (lastCommandSent >= 0 && now - lastCommandSent < 250)
            return {Action::None, 0};
        if (stopRequested) {
            if (!starting && !running() && stopAttempts == 0 && lastTelemetry >= 0 && now - lastTelemetry <= 2000) {
                stopRequested = false;
                return {Action::None, 0};
            }
            if (lastStopSent >= 0 && now - lastStopSent < 1000)
                return {Action::None, 0};
            if (stopAttempts == 6) {
                failure = Action::Stop;
                faulted = true;
                stopRequested = false;
                return {Action::None, 0};
            }
            ++stopAttempts;
            lastStopSent = now;
            lastCommandSent = now;
            return {Action::Stop, 0};
        }
        if (faulted)
            return {Action::None, 0};
        if (starting && now - startSent >= 10000) {
            failure = Action::Start;
            requestStop();
            return next(now);
        }
        if (startRequested) {
            startRequested = false;
            if (!running()) {
                starting = true;
                startSent = now;
                lastCommandSent = now;
                return {Action::Start, 0};
            }
        }
        if (lastTelemetry < 0 || now - lastTelemetry > 2000 || !running() || starting)
            return {Action::None, 0};
        const Command speedCommand = nextTarget(speedTarget, speed, Action::Speed, now);
        if (speedCommand.action != Action::None) {
            lastCommandSent = now;
            return speedCommand;
        }
        const Command inclineCommand = nextTarget(inclineTarget, incline, Action::Incline, now, ypM1552);
        if (inclineCommand.action != Action::None)
            lastCommandSent = now;
        return inclineCommand;
    }

    Action takeFailure() {
        const Action result = failure;
        failure = Action::None;
        return result;
    }

  private:
    struct Target {
        double value = 0;
        bool pending = false;
        unsigned attempts = 0;
        int64_t lastSent = -1;
    };

    bool setTarget(Target &target, double value) {
        // Target changes must never start the belt, or survive a stop request.
        if (stopRequested || faulted || (!running() && !starting && !startRequested))
            return false;
        if (!target.pending || target.value != value) {
            target = Target();
            target.value = value;
            target.pending = true;
        }
        return true;
    }

    Command nextTarget(Target &target, double actual, Action action, int64_t now, bool ypM1552 = false) {
        if (!target.pending)
            return {Action::None, 0};
        double expected = target.value;
        if (ypM1552 && action == Action::Incline) {
            // This firmware truncates the requested level to an integer in its FTMS telemetry.
            // E.g. target 2% sends level 3.5, but the console reports level 3 (about 1.71%).
            const double reportedLevel = std::floor(percentToLevel(target.value) + 1e-6);
            expected = levelToPercent(reportedLevel);
        }
        if (std::fabs(actual - expected) < 0.05) {
            target = Target();
            return {Action::None, 0};
        }
        if (target.lastSent >= 0 && now - target.lastSent < 2500)
            return {Action::None, 0};
        if (target.attempts == 3) {
            target = Target();
            failure = action;
            return {Action::None, 0};
        }
        ++target.attempts;
        target.lastSent = now;
        return {action, target.value};
    }

    Target speedTarget;
    Target inclineTarget;
    bool startRequested = false;
    bool starting = false;
    bool stopRequested = false;
    bool stopAcknowledged = false;
    bool faulted = false;
    unsigned stopAttempts = 0;
    int64_t startSent = 0;
    int64_t lastStopSent = -1;
    int64_t lastCommandSent = -1;
    int64_t lastTelemetry = -1;
    double speed = 0;
    double incline = 0;
    Action failure = Action::None;
};

#endif // YPOOM5CONTROL_H
