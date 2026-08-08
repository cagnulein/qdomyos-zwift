#ifndef RESISTANCESLEWLIMITER_H
#define RESISTANCESLEWLIMITER_H

#include "definitions.h"

#include <QtGlobal>
#include <cmath>

/**
 * @brief Rate limiter for resistance commands.
 *
 * Some bikes move the magnets with a motor that is far slower than the rate at which
 * the app changes its target: measured on a YPOO-based console, the actuator does
 * 2-3 levels/s going up and 4-5 levels/s coming down, while ERG demand arrives about
 * once per second and can span a dozen levels. On those consoles the value published
 * back over FTMS is the commanded target, not the magnet position, so a target that
 * runs ahead of the actuator makes the bike report an effort the rider never made,
 * and the up/down asymmetry biases that error in one direction.
 *
 * This class turns a step command into a ramp: it holds the final target and hands
 * out, one call at a time, the furthest level the actuator could actually have
 * reached since the previous command. With both rates at 0 it is inert and the
 * caller must keep writing the target unchanged.
 *
 * The class is deliberately free of Qt objects and of any clock of its own - the
 * caller passes the timestamp - so it can be unit tested without a device.
 */
class resistanceSlewLimiter {
  public:
    /**
     * @brief Set the maximum command rate for each direction.
     * @param upLevelsPerSecond Levels per second when the target is above the last
     *        command. 0 (or less) leaves increases unlimited.
     * @param downLevelsPerSecond Same, for decreases.
     */
    void configure(double upLevelsPerSecond, double downLevelsPerSecond) {
        m_up = upLevelsPerSecond > 0 ? upLevelsPerSecond : 0.0;
        m_down = downLevelsPerSecond > 0 ? downLevelsPerSecond : 0.0;
    }

    /**
     * @brief True when at least one direction is limited.
     */
    bool enabled() const { return m_up > 0.0 || m_down > 0.0; }

    double upRate() const { return m_up; }
    double downRate() const { return m_down; }

    /**
     * @brief Adopt a level that was commanded outside the limiter.
     *
     * Drops any pending ramp: the caller has just overridden it.
     */
    void sync(resistance_t level) {
        m_lastCommanded = level;
        m_target = level;
        m_known = true;
    }

    /**
     * @brief Forget everything, as after a disconnection.
     */
    void reset() {
        m_lastCommanded = 0;
        m_target = 0;
        m_known = false;
        m_lastStepMs = 0;
    }

    /**
     * @brief Register the final target the caller wants to reach.
     *
     * Re-targeting mid-ramp does not restart the clock, so a stream of ERG updates
     * cannot stall the ramp; starting a ramp from rest does, so the first level of
     * movement is not granted for free out of the idle time.
     */
    void setTarget(resistance_t target, qint64 nowMs) {
        if (!pending())
            m_lastStepMs = nowMs;
        m_target = target;
    }

    /**
     * @brief True while the target has not been handed out yet.
     */
    bool pending() const { return m_known && m_target != m_lastCommanded; }

    /**
     * @brief True once the limiter knows which level the bike is sitting at.
     */
    bool primed() const { return m_known; }

    resistance_t target() const { return m_target; }
    resistance_t lastCommanded() const { return m_lastCommanded; }

    /**
     * @brief Compute the next level to command, if the rate allows one now.
     * @param nowMs Current time, in milliseconds, on any monotonic-enough clock.
     * @param next Receives the level to write.
     * @return true when a command should be issued.
     *
     * The first command after a sync-less start is never limited: there is no
     * previous level to measure a rate against.
     */
    bool step(qint64 nowMs, resistance_t *next) {
        if (!m_known) {
            // Nothing to ramp from - take the target as the starting point.
            m_lastCommanded = m_target;
            m_known = true;
            m_lastStepMs = nowMs;
            *next = m_target;
            return true;
        }

        if (m_target == m_lastCommanded)
            return false;

        const bool goingUp = m_target > m_lastCommanded;
        const double rate = goingUp ? m_up : m_down;
        const int32_t remaining = qAbs((int32_t)m_target - (int32_t)m_lastCommanded);

        int32_t levels;
        if (rate <= 0.0) {
            // This direction is not limited.
            levels = remaining;
        } else {
            qint64 elapsedMs = nowMs - m_lastStepMs;
            if (elapsedMs < 0)
                elapsedMs = 0; // clock went backwards
            if (elapsedMs > kMaxCatchUpMs)
                elapsedMs = kMaxCatchUpMs;

            levels = (int32_t)std::floor(rate * (double)elapsedMs / 1000.0);
            if (levels < 1)
                return false;
            if (levels > remaining)
                levels = remaining;
        }

        if (rate > 0.0) {
            // Charge only the time actually spent, so the leftover fraction of a
            // level is carried into the next call and the average rate stays exact.
            m_lastStepMs += (qint64)std::llround((double)levels * 1000.0 / rate);
        } else {
            m_lastStepMs = nowMs;
        }

        m_lastCommanded = (resistance_t)(goingUp ? (int32_t)m_lastCommanded + levels
                                                 : (int32_t)m_lastCommanded - levels);
        *next = m_lastCommanded;
        return true;
    }

  private:
    /**
     * A ramp that goes unserviced - a stalled poll, a suspended app - must not buy
     * the right to jump. The actuator was not moving during that gap either, so
     * credit is capped at this much elapsed time.
     */
    static constexpr qint64 kMaxCatchUpMs = 2000;

    double m_up = 0.0;
    double m_down = 0.0;
    resistance_t m_lastCommanded = 0;
    resistance_t m_target = 0;
    bool m_known = false;
    qint64 m_lastStepMs = 0;
};

#endif // RESISTANCESLEWLIMITER_H
