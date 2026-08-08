#ifndef FTMSCONTROLPOINTHANDSHAKE_H
#define FTMSCONTROLPOINTHANDSHAKE_H

#include <QtGlobal>

/**
 * @brief The FTMS control point handshake, as a state machine with no clock and no Qt objects.
 *
 * FTMS requires the server to answer REQUEST_CONTROL before it accepts anything else. Writing
 * START_RESUME straight afterwards is a race: if control has not been granted yet the bike
 * discards the start, the console never counts down, and - because the old code latched its
 * "done" flag on the *queueing* of the write rather than on any acknowledgement - the failed
 * handshake was never retried and only an app restart recovered it.
 *
 * So this waits. Each command is issued, then its acknowledgement on the control point is
 * matched by request code before the next one goes out; an unanswered command is reissued.
 *
 * The important compromise: **not every bike answers.** Plenty of FTMS consoles never indicate
 * on 0x2AD9 at all, and gating the handshake on an acknowledgement that will never arrive would
 * break every one of them - a far worse defect than the one being fixed. After kMaxAttempts the
 * machine therefore gives up waiting and carries on as the old code did, blind, flagging itself
 * degraded() so the caller can say so in the log. The floor is the old behaviour; the ceiling is
 * a handshake that is actually correct.
 */
class ftmsControlPointHandshake {
  public:
    /**
     * @brief Start the handshake.
     * @param controlOpcode The request-control opcode to send and match first.
     * @param startOpcode The opcode that starts the session, sent only once control is granted.
     *
     * The caller owns the byte layout: some bikes need a payload alongside the start opcode, and
     * some need a prelude before the handshake. This only sequences the two commands whose
     * ordering the spec constrains.
     */
    void begin(uint8_t controlOpcode, uint8_t startOpcode, qint64 nowMs) {
        Q_UNUSED(nowMs) // nothing is outstanding yet; the clock starts when the first write goes out
        m_controlOpcode = controlOpcode;
        m_startOpcode = startOpcode;
        m_step = Step::RequestControl;
        m_issued = false;
        m_issuedAtMs = 0;
        m_attempts = 0;
        m_degraded = false;
    }

    /**
     * @brief Forget everything, as after a disconnection.
     */
    void reset() {
        m_step = Step::Idle;
        m_issued = false;
        m_issuedAtMs = 0;
        m_attempts = 0;
        m_degraded = false;
    }

    bool active() const { return m_step == Step::RequestControl || m_step == Step::StartResume; }
    bool done() const { return m_step == Step::Done; }

    /**
     * @brief True when the handshake finished without the bike ever acknowledging.
     *
     * Not a failure - the session still runs, exactly as it did before this class existed - but
     * worth one line in the log, because it means every diagnosis of this bike has to treat the
     * control point as write-only.
     */
    bool degraded() const { return m_degraded; }

    int attempts() const { return m_attempts; }

    /**
     * @brief The opcode currently awaiting acknowledgement.
     * @param opcode Receives it, when there is one.
     * @return false when nothing is in flight.
     *
     * Deliberately not a plain getter returning 0 for "none": REQUEST_CONTROL *is* 0x00, so a
     * sentinel would make "waiting for the control grant" and "waiting for nothing" the same
     * value, and any assertion on it vacuous.
     */
    bool pending(uint8_t *opcode) const {
        if (!m_issued)
            return false;
        *opcode = m_step == Step::RequestControl ? m_controlOpcode : m_startOpcode;
        return true;
    }

    /**
     * @brief Ask what to write now.
     * @param opcode Receives the opcode to write.
     * @return true when the caller should issue a write.
     *
     * Returns false while an acknowledgement is still within its window - that wait is the whole
     * point - and true again when the window expires and the command is worth repeating.
     */
    bool nextCommand(qint64 nowMs, uint8_t *opcode) {
        if (!active())
            return false;

        const uint8_t current = m_step == Step::RequestControl ? m_controlOpcode : m_startOpcode;

        if (!m_issued) { // never sent
            m_issued = true;
            m_issuedAtMs = nowMs;
            m_attempts = 1;
            *opcode = current;
            return true;
        }

        qint64 waited = nowMs - m_issuedAtMs;
        if (waited < 0)
            waited = 0; // clock went backwards
        if (waited < kAckTimeoutMs)
            return false;

        if (m_attempts >= kMaxAttempts) {
            // This bike is not going to answer. Proceed as the old code did.
            m_degraded = true;
            advance();
            if (!active())
                return false;
            m_issued = true;
            m_issuedAtMs = nowMs;
            m_attempts = 1;
            *opcode = m_startOpcode;
            return true;
        }

        m_issuedAtMs = nowMs;
        m_attempts++;
        *opcode = current;
        return true;
    }

    /**
     * @brief Feed in a control point response.
     * @param requestCode The opcode the bike is answering about.
     * @param success Whether the result code said success.
     *
     * Responses to anything other than the outstanding command are ignored: resistance writes
     * are acknowledged on the same characteristic and must not be mistaken for progress.
     */
    void onResponse(uint8_t requestCode, bool success, qint64 nowMs) {
        if (!active() || !m_issued)
            return;

        const uint8_t current = m_step == Step::RequestControl ? m_controlOpcode : m_startOpcode;
        if (requestCode != current)
            return;

        if (success) {
            advance();
            return;
        }

        // Rejected. Let the timeout reissue it, unless there is no patience left - in which case
        // carry on regardless, because that is still what the old code would have done.
        if (m_attempts >= kMaxAttempts) {
            m_degraded = true;
            advance();
        } else {
            m_issuedAtMs = nowMs - kAckTimeoutMs; // eligible to resend immediately
        }
    }

    /**
     * @brief The write could not even be queued, so it was never really issued.
     */
    void onWriteFailed() {
        if (!active())
            return;
        m_issued = false;
        if (m_attempts > 0)
            m_attempts--;
    }

  private:
    enum class Step { Idle, RequestControl, StartResume, Done };

    /**
     * How long to wait for an acknowledgement before repeating a command. An FTMS indication
     * comes back in tens of milliseconds when it comes at all, so this is generous on purpose.
     */
    static constexpr qint64 kAckTimeoutMs = 1000;

    /**
     * Attempts per command before concluding this bike does not acknowledge. Three attempts at a
     * one second timeout puts the worst case - a completely silent control point - at three
     * seconds per command, paid once per connection.
     */
    static constexpr int kMaxAttempts = 3;

    void advance() {
        if (m_step == Step::RequestControl)
            m_step = Step::StartResume;
        else if (m_step == Step::StartResume)
            m_step = Step::Done;
        m_issued = false;
        m_issuedAtMs = 0;
        m_attempts = 0;
    }

    Step m_step = Step::Idle;
    bool m_issued = false;
    uint8_t m_controlOpcode = 0;
    uint8_t m_startOpcode = 0;
    qint64 m_issuedAtMs = 0;
    int m_attempts = 0;
    bool m_degraded = false;
};

#endif // FTMSCONTROLPOINTHANDSHAKE_H
