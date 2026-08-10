#ifndef ERGAUTOMODE_H
#define ERGAUTOMODE_H

#include <QtGlobal>
#include <cstdint> // uint16_t: QtGlobal does not pull this in, it only happened to arrive

/**
 * @brief Decides when ERG should be on, from what the training app is actually asking for.
 *
 * The rule is not guesswork: it comes from the control point writes MyWhoosh makes into QZ's
 * virtual bike, in the ride of 7/8. Two opcodes alternate there, and the alternation lines up
 * exactly with the workout blocks:
 *
 *     19:35:07  indoor bike simulation parameters (0x11)   free ride
 *     19:35:45  erg mode (0x05), then every ~5 s           workout block
 *     19:37:10  indoor bike simulation parameters (0x11)   free ride again
 *     19:37:50  erg mode (0x05)                            next block
 *
 * So the app already says which mode it wants; nothing has to be inferred from cadence or power.
 * Three things decide it, and all three are needed:
 *
 *  1. **Which opcode came last.** A simulation write means the app is driving slope, not power.
 *  2. **Whether the target is non-zero.** 673 of the power requests in that ride were for 0 W,
 *     which is the app saying "no target", not "hold zero watts". Taking the opcode alone would
 *     leave ERG latched on through every coast.
 *  3. **Whether it is still fresh.** Target power arrives every ~5 s while a block runs. If it
 *     stops arriving the block is over - or the app is gone - and holding the last resistance
 *     forever is the worst of the three outcomes.
 *
 * No Qt objects and no clock of its own: the caller supplies now, so every branch is reachable
 * from a test.
 */
class ergAutoMode {
  public:
    /** A set-target-power (0x05) write arrived. Zero watts means "no target", not zero watts. */
    void onTargetPower(uint16_t watts, qint64 nowMs) {
        m_lastPowerMs = nowMs;
        m_lastPowerSeen = true;
        m_targetWatts = watts;
        if (watts > 0)
            requestOn(nowMs);
        else
            requestOff(nowMs);
    }

    /** An indoor-bike-simulation (0x11) write arrived: the app wants slope, not power. */
    void onSimulation(qint64 nowMs) { requestOff(nowMs); }

    /**
     * @brief Whether ERG should be engaged right now.
     *
     * Call it on the poll, not only on writes: staleness is a function of time passing, so
     * nothing else can notice that the targets stopped coming.
     */
    bool ergShouldBeOn(qint64 nowMs) const {
        if (!m_on)
            return false;
        if (!m_lastPowerSeen)
            return false;
        const qint64 age = nowMs - m_lastPowerMs;
        if (age < 0)
            return true; // clock went backwards; do not tear down a live session over it
        return age <= kStaleMs;
    }

    /** The last target the app asked for, whether or not ERG is currently engaged. */
    uint16_t targetWatts() const { return m_targetWatts; }

    /** True once a target-power write has ever been seen on this connection. */
    bool everSawTarget() const { return m_lastPowerSeen; }

    void reset() {
        m_on = false;
        m_lastPowerSeen = false;
        m_lastPowerMs = 0;
        m_changedAtMs = 0;
        m_targetWatts = 0;
    }

    /**
     * How long a target power stays good for. They arrive every ~5 s during a block, so this is
     * three missed updates - long enough not to trip on a hiccup, short enough that the end of a
     * workout is noticed before the rider is.
     */
    static constexpr qint64 kStaleMs = 15000;

    /**
     * Minimum time in a state before the opposite request is honoured. The transitions in the
     * log come in bursts - three simulation writes inside 200 ms at 19:37:10 - and a mode that
     * toggles per write would thrash the resistance motor for no reason.
     */
    static constexpr qint64 kMinHoldMs = 3000;

  private:
    void requestOn(qint64 nowMs) { transition(true, nowMs); }
    void requestOff(qint64 nowMs) { transition(false, nowMs); }

    void transition(bool on, qint64 nowMs) {
        if (m_on == on)
            return;
        qint64 held = nowMs - m_changedAtMs;
        if (held < 0)
            held = kMinHoldMs; // clock went backwards: do not let it block a legitimate change
        if (m_changedAtMs != 0 && held < kMinHoldMs)
            return;
        m_on = on;
        m_changedAtMs = nowMs == 0 ? 1 : nowMs; // 0 doubles as "never changed"
    }

    bool m_on = false;
    bool m_lastPowerSeen = false;
    qint64 m_lastPowerMs = 0;
    qint64 m_changedAtMs = 0;
    uint16_t m_targetWatts = 0;
};

#endif // ERGAUTOMODE_H
